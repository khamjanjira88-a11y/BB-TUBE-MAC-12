#pragma once

#include <JuceHeader.h>
#include <array>
#include <atomic>

class BBTubeCompressorAudioProcessor : public juce::AudioProcessor
{
public:
    static constexpr int kFFTOrder = 11;
    static constexpr int kFFTSize = 1 << kFFTOrder;
    static constexpr int kSpectrumBins = kFFTSize / 2;
    static constexpr int kWaveformSize = 1024;
    static constexpr int kHistorySize = 240;

    BBTubeCompressorAudioProcessor();
    ~BBTubeCompressorAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& parameters() noexcept { return apvts; }

    float inputPeakDb() const noexcept { return inputPeak.load(); }
    float outputPeakDb() const noexcept { return outputPeak.load(); }
    float inputRmsDb() const noexcept { return inputRms.load(); }
    float outputRmsDb() const noexcept { return outputRms.load(); }
    float gainReductionDb() const noexcept { return gainReduction.load(); }
    float correlation() const noexcept { return correlationMeter.load(); }
    float balanceDb() const noexcept { return balanceMeter.load(); }
    float shortTermLufs() const noexcept { return shortTermLufsMeter.load(); }
    float integratedLufs() const noexcept { return integratedLufsMeter.load(); }
    float loudnessRange() const noexcept { return loudnessRangeMeter.load(); }
    float samplePeakDb() const noexcept { return samplePeakMeter.load(); }

    void copySpectrum(std::array<float, kSpectrumBins>& dst) const;
    void copyWaveform(std::array<float, kWaveformSize>& dst) const;
    void copyGainHistory(std::array<float, kHistorySize>& dst) const;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void loadFactoryPreset(const juce::String& presetName);

private:
    juce::AudioProcessorValueTreeState apvts;

    double sampleRate = 44100.0;
    int maxBlockSize = 512;

    float detectorEnv = 0.0f;
    float compressorGain = 1.0f;
    float limiterGain = 1.0f;

    juce::dsp::IIR::Filter<float> sidechainHPF;
    juce::dsp::IIR::Filter<float> sidechainLPF;

    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> lookaheadDelay { 4096 };
    int lookaheadSamples = 0;

    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;

    std::atomic<float> inputPeak { -100.0f };
    std::atomic<float> outputPeak { -100.0f };
    std::atomic<float> inputRms { -100.0f };
    std::atomic<float> outputRms { -100.0f };
    std::atomic<float> gainReduction { 0.0f };
    std::atomic<float> correlationMeter { 0.0f };
    std::atomic<float> balanceMeter { 0.0f };
    std::atomic<float> shortTermLufsMeter { -100.0f };
    std::atomic<float> integratedLufsMeter { -100.0f };
    std::atomic<float> loudnessRangeMeter { 0.0f };
    std::atomic<float> samplePeakMeter { -100.0f };

    juce::dsp::FFT fft { kFFTOrder };
    juce::dsp::WindowingFunction<float> window {
        kFFTSize,
        juce::dsp::WindowingFunction<float>::hann,
        true
    };

    mutable juce::SpinLock analyzerLock;
    std::array<float, kFFTSize> fftFifo {};
    std::array<float, kFFTSize * 2> fftData {};
    std::array<float, kSpectrumBins> spectrum {};
    std::array<float, kWaveformSize> waveform {};
    std::array<float, kHistorySize> gainHistory {};
    int fftFifoIndex = 0;
    int waveformWriteIndex = 0;
    int gainHistoryIndex = 0;

    // Real-time loudness approximation. The GUI labels it as an indication;
    // commercial release should replace with full BS.1770/K-weighting/gating.
    double loudnessEnergy = 0.0;
    uint64_t loudnessBlocks = 0;
    std::array<float, 60> loudnessHistory {};
    int loudnessHistoryIndex = 0;

    static float dbToGain(float db) noexcept;
    static float gainToDb(float gain) noexcept;
    static float smoothingCoeff(float ms, double sr) noexcept;
    static float compressionGainDb(float inputDb, float threshold,
                                   float ratio, float knee) noexcept;
    static float tubeShape(float x, float driveDb, float bias,
                           float harmonics) noexcept;

    float readParameter(const char* id) const noexcept;
    void updateAnalyzer(float sample);
    void publishWaveform(const juce::AudioBuffer<float>& buffer);
    void publishGainHistory(float gainReductionDbValue);
    void updateLoudness(float rms);
    void updateFilters();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BBTubeCompressorAudioProcessor)
};
