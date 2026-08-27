#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

namespace
{
constexpr float kEpsilon = 1.0e-7f;

static float softKneeGainDb(float inputDb, float threshold, float ratio, float knee) noexcept
{
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
    using P = juce::AudioParameterFloat;
    using R = juce::NormalisableRange<float>;

    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    auto addFloat = [&params](const char* id, const char* name,
                              float lo, float hi, float def, float step)
    {
        params.push_back(std::make_unique<P>(id, name, R(lo, hi, step), def));
    };

    addFloat("input",         "Input",            -24.0f, 24.0f, 0.0f,   0.01f);
    addFloat("threshold",     "Threshold",        -60.0f, 0.0f, -10.0f, 0.1f);
    addFloat("ratio",         "Ratio",              1.0f, 20.0f, 4.0f,  0.01f);
    addFloat("attack",        "Attack",             0.1f, 100.0f, 10.0f, 0.1f);
    addFloat("release",       "Release",           10.0f, 2000.0f, 100.0f, 1.0f);
    addFloat("knee",          "Knee",               0.0f, 24.0f, 6.0f,  0.1f);
    addFloat("tubeDrive",     "Tube Drive",         0.0f, 24.0f, 3.0f, 0.1f);
    addFloat("tubeBias",      "Tube Bias",         -1.0f, 1.0f, 0.0f,  0.01f);
    addFloat("harmonics",     "Harmonics",          0.0f, 100.0f, 35.0f, 0.1f);
    addFloat("mix",           "Mix",                0.0f, 100.0f, 100.0f, 0.1f);
    addFloat("makeup",        "Makeup",           -12.0f, 18.0f, 3.0f, 0.1f);
    addFloat("stereoLink",    "Stereo Link",        0.0f, 100.0f, 100.0f, 0.1f);
    addFloat("sidechainHPF",  "Sidechain HPF",     20.0f, 1000.0f, 120.0f, 1.0f);
    addFloat("lookahead",     "Lookahead",           0.0f, 10.0f, 2.0f, 0.1f);
    addFloat("limThreshold",  "Limiter Threshold", -12.0f, 0.0f, -1.0f, 0.1f);
    addFloat("ceiling",       "Limiter Ceiling",    -3.0f, 0.0f, -0.3f, 0.01f);
    addFloat("limRelease",    "Limiter Release",    10.0f, 1000.0f, 100.0f, 1.0f);

    params.push_back(std::make_unique<juce::AudioParameterBool>("limiter", "Limiter", true));
    params.push_back(std::make_unique<juce::AudioParameterBool>("truePeak", "True Peak", true));
    params.push_back(std::make_unique<juce::AudioParameterBool>("rms", "RMS Detector", false));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "mode", "Mode",
        juce::StringArray { "Stereo", "Mono", "Mid/Side", "Dual Mono" }, 0));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "style", "Style",
        juce::StringArray { "Opto", "Bus", "FET" }, 0));

    return { params.begin(), params.end() };
}

float BBTubeCompressorAudioProcessor::dbToGain(float db) noexcept
{
    return std::pow(10.0f, db / 20.0f);
}

float BBTubeCompressorAudioProcessor::gainToDb(float gain) noexcept
{
    return 20.0f * std::log10(std::max(gain, kEpsilon));
}

float BBTubeCompressorAudioProcessor::smoothingCoeff(float timeMs, double sr) noexcept
{
    return std::exp(-1.0f / (std::max(0.1f, timeMs) * 0.001f * static_cast<float>(sr)));
}

float BBTubeCompressorAudioProcessor::compressionGainDb(float inputDb, float threshold,
                                                        float ratio, float knee) noexcept
{
    return softKneeGainDb(inputDb, threshold, std::max(1.0f, ratio), knee);
}

float BBTubeCompressorAudioProcessor::tubeShape(float x, float driveDb,
                                                float bias, float harmonics) noexcept
{
    const float drive = dbToGain(driveDb);
    const float shifted = x * drive + bias * 0.08f;
    const float triode = std::tanh(shifted);
    const float normaliser = std::tanh(std::max(1.0f, drive));
    const float shaped = normaliser > 0.0f ? triode / normaliser : x;

    const float h = juce::jlimit(0.0f, 1.0f, harmonics * 0.01f);
    return x * (1.0f - h) + shaped * h;
}

void BBTubeCompressorAudioProcessor::prepareToPlay(double sr, int)
{
    sampleRate = sr;
    detectorEnv = 0.0f;
    compressorGain = 1.0f;
    limiterGain = 1.0f;
}

void BBTubeCompressorAudioProcessor::releaseResources()
{
}

bool BBTubeCompressorAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto in = layouts.getMainInputChannelSet();
    const auto out = layouts.getMainOutputChannelSet();

    if (in != out)
        return false;

    return in == juce::AudioChannelSet::mono()
        || in == juce::AudioChannelSet::stereo();
}

void BBTubeCompressorAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                                  juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const auto get = [this](const char* id)
    {
        return apvts.getRawParameterValue(id)->load();
    };

    const float inputDb      = get("input");
    const float threshold    = get("threshold");
    const float ratio        = get("ratio");
    const float attackMs     = get("attack");
    const float releaseMs    = get("release");
    const float knee         = get("knee");
    const float tubeDrive    = get("tubeDrive");
    const float tubeBias     = get("tubeBias");
    const float harmonics    = get("harmonics");
    const float mix          = get("mix") * 0.01f;
    const float makeupDb     = get("makeup");
    const float stereoLink   = get("stereoLink") * 0.01f;
    const float limitThresh  = get("limThreshold");
    const float ceilingDb    = get("ceiling");
    const float limReleaseMs = get("limRelease");
    const bool limiterOn     = get("limiter") > 0.5f;
    const bool rmsMode       = get("rms") > 0.5f;
    const int mode            = static_cast<int>(get("mode"));
    const int style           = static_cast<int>(get("style"));

    const float inGain = dbToGain(inputDb);
    const float makeup = dbToGain(makeupDb);
    const float attackCoeff = smoothingCoeff(attackMs, sampleRate);
    const float releaseCoeff = smoothingCoeff(releaseMs, sampleRate);
    const float limReleaseCoeff = smoothingCoeff(limReleaseMs, sampleRate);

    float maxIn = 0.0f;
    float maxOut = 0.0f;
    float maxGR = 0.0f;

    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    for (int i = 0; i < numSamples; ++i)
    {
        float left = buffer.getSample(0, i) * inGain;
        float right = numChannels > 1 ? buffer.getSample(1, i) * inGain : left;

        maxIn = std::max(maxIn, std::max(std::abs(left), std::abs(right)));

        // Original tube coloration first.
        left = tubeShape(left, tubeDrive, tubeBias, harmonics);
        right = tubeShape(right, tubeDrive, tubeBias, harmonics);

        float detector = std::max(std::abs(left), std::abs(right));

        if (mode == 1) // mono
            detector = std::max(std::abs(left), std::abs(right));
        else if (mode == 2) // M/S
        {
            const float mid  = (left + right) * 0.70710678f;
            const float side = (left - right) * 0.70710678f;
            detector = std::max(std::abs(mid), std::abs(side));
        }
        else if (mode == 3) // dual mono
            detector = std::abs(left);

        if (rmsMode)
            detectorEnv = std::sqrt(0.92f * detectorEnv * detectorEnv + 0.08f * detector * detector);
        else
            detectorEnv = detector;

        const float detectorDb = gainToDb(detectorEnv);
        float targetGrDb = compressionGainDb(detectorDb, threshold, ratio, knee);

        // Style adds subtle response differences without emulating proprietary circuitry.
        if (style == 1)
            targetGrDb *= 1.05f;
        else if (style == 2)
            targetGrDb *= 0.90f;

        const float targetGain = dbToGain(targetGrDb);

        if (targetGain < compressorGain)
            compressorGain = attackCoeff * compressorGain + (1.0f - attackCoeff) * targetGain;
        else
            compressorGain = releaseCoeff * compressorGain + (1.0f - releaseCoeff) * targetGain;

        float wetLeft = left * compressorGain * makeup;
        float wetRight = right * compressorGain * makeup;

        if (mode == 3 && numChannels > 1)
        {
            const float leftDb = gainToDb(std::abs(left));
            const float rightDb = gainToDb(std::abs(right));
            const float gl = dbToGain(compressionGainDb(leftDb, threshold, ratio, knee));
            const float gr = dbToGain(compressionGainDb(rightDb, threshold, ratio, knee));
            wetLeft = left * gl * makeup;
            wetRight = right * gr * makeup;
        }

        // Stereo Link blends linked detector gain with independent peak information.
        if (mode == 0 && numChannels > 1 && stereoLink < 0.999f)
        {
            const float lDb = gainToDb(std::abs(left));
            const float rDb = gainToDb(std::abs(right));
            const float lg = dbToGain(compressionGainDb(lDb, threshold, ratio, knee));
            const float rg = dbToGain(compressionGainDb(rDb, threshold, ratio, knee));
            const float linkedGainL = compressorGain * (1.0f - stereoLink) + lg * stereoLink;
            const float linkedGainR = compressorGain * (1.0f - stereoLink) + rg * stereoLink;
            wetLeft = left * linkedGainL * makeup;
            wetRight = right * linkedGainR * makeup;
        }

        if (limiterOn)
        {
            const float peak = std::max(std::abs(wetLeft), std::abs(wetRight));
            const float ceiling = dbToGain(ceilingDb);
            const float desired = peak > ceiling ? ceiling / std::max(peak, kEpsilon) : 1.0f;

            if (desired < limiterGain)
                limiterGain = desired;
            else
                limiterGain = limReleaseCoeff * limiterGain
                            + (1.0f - limReleaseCoeff) * desired;

            wetLeft *= limiterGain;
            wetRight *= limiterGain;

            // Limit target provides an additional conservative clamp.
            if (gainToDb(std::max(std::abs(wetLeft), std::abs(wetRight))) > limitThresh)
            {
                const float hard = dbToGain(ceilingDb)
                                  / std::max(std::abs(wetLeft), std::abs(wetRight));
                wetLeft *= std::min(1.0f, hard);
                wetRight *= std::min(1.0f, hard);
            }
        }

        const float outLeft = left * (1.0f - mix) + wetLeft * mix;
        const float outRight = right * (1.0f - mix) + wetRight * mix;

        buffer.setSample(0, i, outLeft);
        if (numChannels > 1)
            buffer.setSample(1, i, outRight);

        maxOut = std::max(maxOut, std::max(std::abs(outLeft), std::abs(outRight)));

        const float gr = std::max(0.0f, -gainToDb(compressorGain));
        maxGR = std::max(maxGR, gr);
    }

    inputPeakDb.store(gainToDb(maxIn));
    outputPeakDb.store(gainToDb(maxOut));
    gainReductionDb.store(maxGR);
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
