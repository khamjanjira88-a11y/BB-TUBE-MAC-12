#pragma once
#include <JuceHeader.h>

class BBTubeCompressorAudioProcessor : public juce::AudioProcessor
{
public:
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

    float getInputPeakDb() const noexcept { return inputPeakDb.load(); }
    float getOutputPeakDb() const noexcept { return outputPeakDb.load(); }
    float getGainReductionDb() const noexcept { return gainReductionDb.load(); }

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    juce::AudioProcessorValueTreeState apvts;

    double sampleRate = 44100.0;
    float detectorEnv = 0.0f;
    float compressorGain = 1.0f;
    float limiterGain = 1.0f;

    std::atomic<float> inputPeakDb { -100.0f };
    std::atomic<float> outputPeakDb { -100.0f };
    std::atomic<float> gainReductionDb { 0.0f };

    static float dbToGain(float db) noexcept;
    static float gainToDb(float gain) noexcept;
    static float smoothingCoeff(float timeMs, double sr) noexcept;
    static float compressionGainDb(float inputDb, float threshold, float ratio, float knee) noexcept;
    static float tubeShape(float x, float driveDb, float bias, float harmonics) noexcept;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BBTubeCompressorAudioProcessor)
};
