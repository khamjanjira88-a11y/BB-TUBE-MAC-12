#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

namespace
{
constexpr float kEpsilon = 1.0e-7f;

float dbToGainLocal(float db) noexcept
{
    return std::pow(10.0f, db / 20.0f);
}

float gainToDbLocal(float gain) noexcept
{
    return 20.0f * std::log10(std::max(gain, kEpsilon));
}

float softKneeGainDb(float inputDb, float threshold, float ratio, float knee) noexcept
{
    ratio = std::max(1.0f, ratio);
    if (knee <= 0.0f)
        return inputDb > threshold
            ? (threshold + (inputDb - threshold) / ratio) - inputDb
            : 0.0f;

    const float lower = threshold - knee * 0.5f;
    const float upper = threshold + knee * 0.5f;

    if (inputDb <= lower)
        return 0.0f;
    if (inputDb >= upper)
        return (threshold + (inputDb - threshold) / ratio) - inputDb;

    const float x = inputDb - lower;
    const float amount = (1.0f / ratio - 1.0f);
    return amount * x * x / (2.0f * knee);
}

float rmsToLufsApprox(float rms) noexcept
{
    return gainToDbLocal(std::max(rms, kEpsilon)) - 0.691f;
}
}

BBTubeCompressorAudioProcessor::BBTubeCompressorAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "BB_STATE", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout
BBTubeCompressorAudioProcessor::createParameterLayout()
{
    using F = juce::AudioParameterFloat;
    using B = juce::AudioParameterBool;
    using C = juce::AudioParameterChoice;
    using R = juce::NormalisableRange<float>;

    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
    auto add = [&p](const char* id, const char* name,
                    float lo, float hi, float def, float step)
    {
        p.push_back(std::make_unique<F>(id, name, R(lo, hi, step), def));
    };

    add("input", "Input", -24.0f, 24.0f, 0.0f, 0.01f);
    add("output", "Output", -24.0f, 24.0f, 0.0f, 0.01f);
    add("threshold", "Threshold", -60.0f, 0.0f, -10.0f, 0.1f);
    add("ratio", "Ratio", 1.0f, 20.0f, 4.0f, 0.01f);
    add("attack", "Attack", 0.1f, 100.0f, 10.0f, 0.1f);
    add("release", "Release", 10.0f, 2000.0f, 100.0f, 1.0f);
    add("knee", "Knee", 0.0f, 24.0f, 6.0f, 0.1f);
    add("mix", "Mix", 0.0f, 100.0f, 100.0f, 0.1f);
    add("stereoLink", "Stereo Link", 0.0f, 100.0f, 100.0f, 0.1f);
    add("makeup", "Makeup Gain", -24.0f, 24.0f, 3.0f, 0.1f);
    add("tubeDrive", "Tube Drive", 0.0f, 24.0f, 3.0f, 0.1f);
    add("tubeBias", "Tube Bias", -1.0f, 1.0f, 0.0f, 0.01f);
    add("harmonics", "Harmonics", 0.0f, 100.0f, 35.0f, 0.1f);
    add("scHPF", "Sidechain HPF", 20.0f, 1000.0f, 120.0f, 1.0f);
    add("scLPF", "Sidechain LPF", 1000.0f, 20000.0f, 20000.0f, 10.0f);
    add("lookahead", "Lookahead", 0.0f, 20.0f, 2.0f, 0.1f);
    add("limThreshold", "Limiter Threshold", -12.0f, 0.0f, -1.0f, 0.1f);
    add("ceiling", "Ceiling", -3.0f, 0.0f, -0.3f, 0.01f);
    add("limRelease", "Limiter Release", 10.0f, 1000.0f, 100.0f, 1.0f);

    p.push_back(std::make_unique<B>("limiter", "Limiter", true));
    p.push_back(std::make_unique<B>("truePeak", "True Peak", true));
    p.push_back(std::make_unique<B>("rms", "RMS Detector", false));
    p.push_back(std::make_unique<B>("autoMakeup", "Auto Makeup", false));
    p.push_back(std::make_unique<B>("warmth", "Warmth", true));

    p.push_back(std::make_unique<C>("mode", "Mode",
        juce::StringArray { "Stereo", "Mono", "Mid / Side", "Dual Mono" }, 0));
    p.push_back(std::make_unique<C>("style", "Compressor Style",
        juce::StringArray { "Opto", "VCA", "FET" }, 0));
    p.push_back(std::make_unique<C>("detector", "Detector",
        juce::StringArray { "Peak", "RMS" }, 0));
    p.push_back(std::make_unique<C>("thrust", "Thrust",
        juce::StringArray { "Clean", "Warm", "Hot" }, 1));
    p.push_back(std::make_unique<C>("oversampling", "Oversampling",
        juce::StringArray { "Off", "2x", "4x", "8x" }, 2));
    p.push_back(std::make_unique<C>("slope", "Filter Slope",
        juce::StringArray { "12 dB", "24 dB" }, 1));

    return { p.begin(), p.end() };
}

float BBTubeCompressorAudioProcessor::readParameter(const char* id) const noexcept
{
    if (auto* v = apvts.getRawParameterValue(id))
        return v->load();
    return 0.0f;
}

float BBTubeCompressorAudioProcessor::dbToGain(float db) noexcept { return dbToGainLocal(db); }
float BBTubeCompressorAudioProcessor::gainToDb(float gain) noexcept { return gainToDbLocal(gain); }

float BBTubeCompressorAudioProcessor::smoothingCoeff(float ms, double sr) noexcept
{
    return std::exp(-1.0f / (std::max(0.1f, ms) * 0.001f * static_cast<float>(sr)));
}

float BBTubeCompressorAudioProcessor::compressionGainDb(
    float inputDb, float threshold, float ratio, float knee) noexcept
{
    return softKneeGainDb(inputDb, threshold, ratio, knee);
}

float BBTubeCompressorAudioProcessor::tubeShape(
    float x, float driveDb, float bias, float harmonics) noexcept
{
    const float drive = dbToGainLocal(driveDb);
    const float biased = x * drive + bias * 0.08f;
    const float soft = std::tanh(biased);
    const float normalizer = std::max(0.15f, std::tanh(std::max(1.0f, drive)));
    const float shaped = soft / normalizer;
    const float amount = juce::jlimit(0.0f, 1.0f, harmonics * 0.01f);
    return x * (1.0f - amount) + shaped * amount;
}

void BBTubeCompressorAudioProcessor::updateFilters()
{
    const float hpf = readParameter("scHPF");
    const float lpf = readParameter("scLPF");

    sidechainHPF.coefficients =
        juce::dsp::IIR::Coefficients<float>::makeHighPass(
            sampleRate, juce::jlimit(20.0f, 1000.0f, hpf));

    sidechainLPF.coefficients =
        juce::dsp::IIR::Coefficients<float>::makeLowPass(
            sampleRate, juce::jlimit(1000.0f, 20000.0f, lpf));
}

void BBTubeCompressorAudioProcessor::prepareToPlay(
    double newRate, int samplesPerBlock)
{
    sampleRate = newRate;
    maxBlockSize = samplesPerBlock;
    detectorEnv = 0.0f;
    compressorGain = 1.0f;
    limiterGain = 1.0f;

    sidechainHPF.reset();
    sidechainLPF.reset();
    updateFilters();

    lookaheadDelay.prepare({ sampleRate, (juce::uint32)samplesPerBlock, 2 });
    lookaheadDelay.setMaximumDelayInSamples(static_cast<int>(sampleRate * 0.02) + 8);
    lookaheadDelay.reset();

    oversampler = std::make_unique<juce::dsp::Oversampling<float>>(
        2, 3,
        juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,
        true);

    fftFifo.fill(0.0f);
    fftData.fill(0.0f);
    spectrum.fill(-120.0f);
    waveform.fill(0.0f);
    gainHistory.fill(0.0f);
    fftFifoIndex = 0;
    waveformWriteIndex = 0;
    gainHistoryIndex = 0;

    loudnessEnergy = 0.0;
    loudnessBlocks = 0;
    loudnessHistory.fill(-100.0f);
    loudnessHistoryIndex = 0;
}

void BBTubeCompressorAudioProcessor::releaseResources()
{
    lookaheadDelay.reset();
}

bool BBTubeCompressorAudioProcessor::isBusesLayoutSupported(
    const BusesLayout& layouts) const
{
    const auto input = layouts.getMainInputChannelSet();
    const auto output = layouts.getMainOutputChannelSet();

    if (input != output)
        return false;

    return input == juce::AudioChannelSet::mono()
        || input == juce::AudioChannelSet::stereo();
}

void BBTubeCompressorAudioProcessor::processBlock(
    juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int channels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    if (channels <= 0 || numSamples <= 0)
        return;

    updateFilters();

    const float inputGain = dbToGainLocal(readParameter("input"));
    const float outputGain = dbToGainLocal(readParameter("output"));
    const float threshold = readParameter("threshold");
    const float ratio = readParameter("ratio");
    const float attack = readParameter("attack");
    const float release = readParameter("release");
    const float knee = readParameter("knee");
    const float mix = readParameter("mix") * 0.01f;
    const float link = readParameter("stereoLink") * 0.01f;
    const float makeup = dbToGainLocal(readParameter("makeup"));
    const float tubeDrive = readParameter("tubeDrive");
    const float tubeBias = readParameter("tubeBias");
    const float harmonics = readParameter("harmonics");
    const bool limiterOn = readParameter("limiter") > 0.5f;
    const bool rmsMode = readParameter("rms") > 0.5f ||
                         static_cast<int>(readParameter("detector")) == 1;
    const bool autoMakeup = readParameter("autoMakeup") > 0.5f;
    const int mode = static_cast<int>(readParameter("mode"));
    const int style = static_cast<int>(readParameter("style"));
    const int thrust = static_cast<int>(readParameter("thrust"));

    const float attackCoeff = smoothingCoeff(attack, sampleRate);
    const float releaseCoeff = smoothingCoeff(release, sampleRate);
    const float limitReleaseCoeff = smoothingCoeff(readParameter("limRelease"), sampleRate);

    lookaheadSamples = juce::jlimit(
        0,
        static_cast<int>(sampleRate * 0.02),
        static_cast<int>(readParameter("lookahead") * 0.001 * sampleRate));

    float maxIn = 0.0f;
    float maxOut = 0.0f;
    double sumIn = 0.0;
    double sumOut = 0.0;
    double leftEnergy = 0.0;
    double rightEnergy = 0.0;
    double lrEnergy = 0.0;
    float maxGR = 0.0f;

    for (int i = 0; i < numSamples; ++i)
    {
        float l = buffer.getSample(0, i) * inputGain;
        float r = channels > 1 ? buffer.getSample(1, i) * inputGain : l;

        maxIn = std::max(maxIn, std::max(std::abs(l), std::abs(r)));

        const float analyzerSample = 0.5f * (l + r);
        updateAnalyzer(analyzerSample);

        l = tubeShape(l, tubeDrive, tubeBias, harmonics);
        r = tubeShape(r, tubeDrive, tubeBias, harmonics);

        float scL = sidechainHPF.processSample(l);
        float scR = sidechainHPF.processSample(r);
        scL = sidechainLPF.processSample(scL);
        scR = sidechainLPF.processSample(scR);

        float detector = std::max(std::abs(scL), std::abs(scR));

        if (mode == 2)
        {
            const float mid = (scL + scR) * 0.70710678f;
            const float side = (scL - scR) * 0.70710678f;
            detector = std::max(std::abs(mid), std::abs(side));
        }
        else if (mode == 3)
        {
            detector = std::abs(scL);
        }

        if (rmsMode)
        {
            detectorEnv = std::sqrt(
                0.94f * detectorEnv * detectorEnv +
                0.06f * detector * detector);
            detector = detectorEnv;
        }

        float grDb = softKneeGainDb(
            gainToDbLocal(detector), threshold, ratio, knee);

        if (style == 1) grDb *= 1.05f;
        else if (style == 2) grDb *= 0.92f;

        if (thrust == 1) grDb *= 0.95f;
        else if (thrust == 2) grDb *= 1.08f;

        const float targetGain = dbToGainLocal(grDb);

        if (targetGain < compressorGain)
            compressorGain = attackCoeff * compressorGain +
                             (1.0f - attackCoeff) * targetGain;
        else
            compressorGain = releaseCoeff * compressorGain +
                             (1.0f - releaseCoeff) * targetGain;

        float effectiveMakeup = makeup;
        if (autoMakeup)
            effectiveMakeup *= dbToGainLocal(std::min(12.0f, -grDb * 0.55f));

        float wetL = l * compressorGain * effectiveMakeup;
        float wetR = r * compressorGain * effectiveMakeup;

        if (mode == 0 && channels > 1 && link < 0.999f)
        {
            const float leftGain = dbToGainLocal(
                softKneeGainDb(gainToDbLocal(std::abs(scL)), threshold, ratio, knee));
            const float rightGain = dbToGainLocal(
                softKneeGainDb(gainToDbLocal(std::abs(scR)), threshold, ratio, knee));

            const float linkedL = compressorGain * link + leftGain * (1.0f - link);
            const float linkedR = compressorGain * link + rightGain * (1.0f - link);

            wetL = l * linkedL * effectiveMakeup;
            wetR = r * linkedR * effectiveMakeup;
        }

        // Real delay-line lookahead: dry path is left untouched, wet path is delayed.
        if (lookaheadSamples > 0)
        {
            lookaheadDelay.pushSample(0, wetL);
            const float delayedL = lookaheadDelay.popSample(0, lookaheadSamples);
            wetL = delayedL;

            if (channels > 1)
            {
                lookaheadDelay.pushSample(1, wetR);
                wetR = lookaheadDelay.popSample(1, lookaheadSamples);
            }
        }

        if (limiterOn)
        {
            const float peak = std::max(std::abs(wetL), std::abs(wetR));
            const float ceilingGain = dbToGainLocal(readParameter("ceiling"));
            const float desired = peak > ceilingGain
                ? ceilingGain / std::max(peak, kEpsilon)
                : 1.0f;

            if (desired < limiterGain)
                limiterGain = desired;
            else
                limiterGain = limitReleaseCoeff * limiterGain +
                              (1.0f - limitReleaseCoeff) * desired;

            wetL *= limiterGain;
            wetR *= limiterGain;

            const float clamp = ceilingGain /
                std::max(std::max(std::abs(wetL), std::abs(wetR)), kEpsilon);

            if (clamp < 1.0f)
            {
                wetL *= clamp;
                wetR *= clamp;
            }
        }

        const float outL = (l * (1.0f - mix) + wetL * mix) * outputGain;
        const float outR = (r * (1.0f - mix) + wetR * mix) * outputGain;

        buffer.setSample(0, i, outL);
        if (channels > 1)
            buffer.setSample(1, i, outR);

        maxOut = std::max(maxOut, std::max(std::abs(outL), std::abs(outR)));
        sumIn += 0.5 * (double(l) * l + double(r) * r);
        sumOut += 0.5 * (double(outL) * outL + double(outR) * outR);
        leftEnergy += double(l) * l;
        rightEnergy += double(r) * r;
        lrEnergy += double(l) * r;
        maxGR = std::max(maxGR, std::max(0.0f, -gainToDbLocal(compressorGain)));
    }

    const double invN = 1.0 / static_cast<double>(std::max(1, numSamples));
    const float inRms = static_cast<float>(std::sqrt(sumIn * invN));
    const float outRms = static_cast<float>(std::sqrt(sumOut * invN));

    float corr = 0.0f;
    if (leftEnergy > kEpsilon && rightEnergy > kEpsilon)
        corr = static_cast<float>(lrEnergy / std::sqrt(leftEnergy * rightEnergy));

    const float leftRms = static_cast<float>(std::sqrt(leftEnergy * invN));
    const float rightRms = static_cast<float>(std::sqrt(rightEnergy * invN));
    const float balance = gainToDbLocal((leftRms + kEpsilon) / (rightRms + kEpsilon));

    inputPeak.store(gainToDbLocal(maxIn));
    outputPeak.store(gainToDbLocal(maxOut));
    inputRms.store(gainToDbLocal(inRms));
    outputRms.store(gainToDbLocal(outRms));
    gainReduction.store(maxGR);
    correlationMeter.store(juce::jlimit(-1.0f, 1.0f, corr));
    balanceMeter.store(juce::jlimit(-24.0f, 24.0f, balance));
    samplePeakMeter.store(gainToDbLocal(maxOut));

    updateLoudness(outRms);
    publishGainHistory(maxGR);
    publishWaveform(buffer);
}

void BBTubeCompressorAudioProcessor::updateAnalyzer(float sample)
{
    if (fftFifoIndex < kFFTSize)
        fftFifo[(size_t)fftFifoIndex++] = sample;

    if (fftFifoIndex < kFFTSize)
        return;

    fftData.fill(0.0f);
    std::copy(fftFifo.begin(), fftFifo.end(), fftData.begin());
    window.multiplyWithWindowingTable(fftData.data(), kFFTSize);
    fft.performFrequencyOnlyForwardTransform(fftData.data());

    std::array<float, kSpectrumBins> local {};
    for (int i = 0; i < kSpectrumBins; ++i)
    {
        const float magnitude = fftData[(size_t)i] /
                                static_cast<float>(kFFTSize);
        local[(size_t)i] = juce::jlimit(-120.0f, 6.0f,
                                        gainToDbLocal(magnitude));
    }

    {
        const juce::SpinLock::ScopedLockType lock(analyzerLock);
        spectrum = local;
    }

    fftFifoIndex = 0;
}

void BBTubeCompressorAudioProcessor::publishWaveform(
    const juce::AudioBuffer<float>& buffer)
{
    const int n = buffer.getNumSamples();
    if (n <= 0)
        return;

    std::array<float, kWaveformSize> local {};
    const int channels = buffer.getNumChannels();

    for (int i = 0; i < kWaveformSize; ++i)
    {
        const int source = juce::jlimit(
            0, n - 1,
            static_cast<int>((static_cast<double>(i) / kWaveformSize) * n));

        const float l = buffer.getSample(0, source);
        const float r = channels > 1 ? buffer.getSample(1, source) : l;
        local[(size_t)i] = 0.5f * (l + r);
    }

    const juce::SpinLock::ScopedLockType lock(analyzerLock);
    waveform = local;
}

void BBTubeCompressorAudioProcessor::publishGainHistory(float gr)
{
    const juce::SpinLock::ScopedLockType lock(analyzerLock);
    gainHistory[(size_t)gainHistoryIndex] = gr;
    gainHistoryIndex = (gainHistoryIndex + 1) % kHistorySize;
}

void BBTubeCompressorAudioProcessor::updateLoudness(float rms)
{
    const float lufs = rmsToLufsApprox(rms);
    shortTermLufsMeter.store(lufs);

    loudnessEnergy += static_cast<double>(rms) * rms;
    ++loudnessBlocks;

    if (loudnessBlocks > 0)
    {
        const float integrated = rmsToLufsApprox(
            static_cast<float>(std::sqrt(loudnessEnergy /
                                         static_cast<double>(loudnessBlocks))));
        integratedLufsMeter.store(integrated);
    }

    loudnessHistory[(size_t)loudnessHistoryIndex] = lufs;
    loudnessHistoryIndex = (loudnessHistoryIndex + 1) % static_cast<int>(loudnessHistory.size());

    float minValue = 1000.0f;
    float maxValue = -1000.0f;
    for (const float value : loudnessHistory)
    {
        minValue = std::min(minValue, value);
        maxValue = std::max(maxValue, value);
    }

    loudnessRangeMeter.store(std::max(0.0f, maxValue - minValue));
}

void BBTubeCompressorAudioProcessor::copySpectrum(
    std::array<float, kSpectrumBins>& dst) const
{
    const juce::SpinLock::ScopedTryLockType lock(analyzerLock);
    if (lock.isLocked())
        dst = spectrum;
}

void BBTubeCompressorAudioProcessor::copyWaveform(
    std::array<float, kWaveformSize>& dst) const
{
    const juce::SpinLock::ScopedTryLockType lock(analyzerLock);
    if (lock.isLocked())
        dst = waveform;
}

void BBTubeCompressorAudioProcessor::copyGainHistory(
    std::array<float, kHistorySize>& dst) const
{
    const juce::SpinLock::ScopedTryLockType lock(analyzerLock);
    if (!lock.isLocked())
        return;

    for (int i = 0; i < kHistorySize; ++i)
    {
        const int idx = (gainHistoryIndex + i) % kHistorySize;
        dst[(size_t)i] = gainHistory[(size_t)idx];
    }
}

void BBTubeCompressorAudioProcessor::loadFactoryPreset(const juce::String& name)
{
    struct Preset { float th, ratio, attack, release, knee, tube, bias, harm, mix, makeup, link, scHPF, scLPF, lookahead, limTh, ceiling, limRelease; };
    Preset p { -10,4,10,100,6,3,0,35,100,3,100,120,20000,2,-1,-0.3f,100 };
    const auto n = name.toLowerCase();

    if (n == "vocal warm") p = {-18,3,8,180,8,7,0.05f,45,100,4,100,120,20000,2,-1,-0.3f,120};
    else if (n == "vocal aggressive") p = {-20,8,3,80,4,9,0.08f,60,100,5,100,180,18000,1,-2,-0.5f,80};
    else if (n == "vocal ballad") p = {-16,2.5f,20,260,10,5,0.03f,35,100,3,100,100,18000,3,-1,-0.5f,180};
    else if (n == "bass warm") p = {-15,4,15,160,6,4,0,35,100,3,100,90,14000,2,-1,-0.3f,160};
    else if (n == "bass punch") p = {-14,5,8,110,4,3,0,25,100,2,100,100,16000,1,-2,-0.5f,120};
    else if (n == "kick drum") p = {-12,5,4,80,3,2,0,15,100,2,100,100,12000,1,-2,-0.5f,80};
    else if (n == "snare glue") p = {-12,4,12,110,5,2,0,20,100,2,100,150,16000,1,-2,-0.5f,110};
    else if (n == "drum bus glue") p = {-12,4,20,120,4,2,0,20,85,2,100,90,18000,2,-1,-0.4f,120};
    else if (n == "drum crush") p = {-20,10,3,70,3,8,0.05f,55,55,6,100,140,16000,1,-3,-0.6f,80};
    else if (n == "mix bus glue") p = {-8,2,30,250,10,1,0,10,70,1,100,100,18000,3,-1,-0.3f,250};
    else if (n == "mix bus warm") p = {-10,2.5f,25,220,8,2,0.02f,20,80,2,100,100,18000,3,-1,-0.3f,220};
    else if (n == "master gentle") p = {-6,1.5f,40,300,12,0.5f,0,5,100,0,100,80,20000,4,-1,-0.2f,300};
    else if (n == "master punch") p = {-8,2,20,180,8,1,0,10,100,0,100,120,18000,2,-1,-0.2f,180};
    else if (n == "master limiter") p = {-4,1.2f,10,150,4,0,0,0,100,0,100,120,20000,4,-2,-0.1f,150};
    else if (n == "acoustic guitar") p = {-14,3,12,150,8,2,0,20,100,2,100,120,18000,2,-1,-0.4f,150};
    else if (n == "piano") p = {-12,2.5f,18,240,8,1,0,10,100,1,100,100,18000,3,-1,-0.4f,240};
    else if (n == "guitar lead") p = {-15,5,6,90,5,4,0.02f,35,100,3,100,160,18000,1,-2,-0.5f,90};
    else if (n == "synth glue") p = {-10,3,10,120,6,3,0,25,100,2,100,180,18000,2,-1,-0.3f,120};
    else if (n == "vintage vibe") p = {-14,3,15,220,10,8,0.04f,55,100,3,100,120,18000,2,-1,-0.4f,220};
    else if (n == "heavy tube") p = {-22,8,2,70,4,12,0.08f,75,100,5,100,180,18000,1,-3,-0.6f,80};

    const auto set = [this](const char* id, float value)
    {
        if (auto* param = apvts.getParameter(id))
        {
            const auto range = apvts.getParameterRange(id);
            param->setValueNotifyingHost(range.convertTo0to1(value));
        }
    };

    set("threshold", p.th); set("ratio", p.ratio); set("attack", p.attack); set("release", p.release);
    set("knee", p.knee); set("tubeDrive", p.tube); set("tubeBias", p.bias); set("harmonics", p.harm);
    set("mix", p.mix); set("makeup", p.makeup); set("stereoLink", p.link); set("scHPF", p.scHPF);
    set("scLPF", p.scLPF); set("lookahead", p.lookahead); set("limThreshold", p.limTh); set("ceiling", p.ceiling);
    set("limRelease", p.limRelease);
}

void BBTubeCompressorAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary(*xml, destData);
}

void BBTubeCompressorAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
    {
        const auto state = juce::ValueTree::fromXml(*xml);
        if (state.isValid())
            apvts.replaceState(state);
    }
}

juce::AudioProcessorEditor* BBTubeCompressorAudioProcessor::createEditor()
{
    return new BBTubeCompressorAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BBTubeCompressorAudioProcessor();
}
