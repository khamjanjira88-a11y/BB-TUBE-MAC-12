#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <array>
#include <memory>

class BBTubeCompressorAudioProcessorEditor : public juce::AudioProcessorEditor,
                                               private juce::Timer
{
public:
    explicit BBTubeCompressorAudioProcessorEditor(BBTubeCompressorAudioProcessor&);
    ~BBTubeCompressorAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    BBTubeCompressorAudioProcessor& processor;

    class BBLookAndFeel;
    std::unique_ptr<BBLookAndFeel> lookAndFeel;

    juce::ComboBox presetBox;
    juce::ComboBox styleBox;
    juce::ComboBox modeBox;
    juce::ComboBox detectorBox;
    juce::ComboBox thrustBox;
    juce::ComboBox slopeBox;
    juce::ComboBox oversamplingBox;

    juce::TextButton compareButton { "COMPARE" };
    juce::TextButton abButton { "A/B" };
    juce::TextButton saveButton { "SAVE" };
    juce::TextButton resetButton { "RESET" };

    juce::ToggleButton limiterButton;
    juce::ToggleButton truePeakButton;
    juce::ToggleButton rmsButton;
    juce::ToggleButton autoMakeupButton;
    juce::ToggleButton warmthButton;

    juce::Slider input, output, threshold, ratio, attack, release, knee;
    juce::Slider mix, stereoLink, makeup;
    juce::Slider tubeDrive, tubeBias, harmonics;
    juce::Slider scHPF, scLPF, lookahead;
    juce::Slider limThreshold, ceiling, limRelease;

    std::unique_ptr<SliderAttachment> aInput, aOutput, aThreshold, aRatio, aAttack, aRelease, aKnee;
    std::unique_ptr<SliderAttachment> aMix, aStereoLink, aMakeup;
    std::unique_ptr<SliderAttachment> aTubeDrive, aTubeBias, aHarmonics;
    std::unique_ptr<SliderAttachment> aScHPF, aScLPF, aLookahead;
    std::unique_ptr<SliderAttachment> aLimThreshold, aCeiling, aLimRelease;
    std::unique_ptr<ButtonAttachment> aLimiter, aTruePeak, aRms, aAutoMakeup, aWarmth;
    std::unique_ptr<ComboAttachment> aStyle, aMode, aDetector, aThrust, aSlope, aOversampling;

    std::array<float, BBTubeCompressorAudioProcessor::kSpectrumBins> spectrum {};
    std::array<float, BBTubeCompressorAudioProcessor::kWaveformSize> waveform {};
    std::array<float, BBTubeCompressorAudioProcessor::kHistorySize> gainHistory {};

    std::unique_ptr<juce::XmlElement> stateA;
    std::unique_ptr<juce::XmlElement> stateB;
    bool usingB = false;

    void setupSlider(juce::Slider&, const juce::String& suffix);
    void addAttachment(juce::Slider&, const char*, std::unique_ptr<SliderAttachment>&);
    void addSectionLabel(juce::Graphics&, const juce::Rectangle<int>&, const juce::String&) const;
    void drawPanel(juce::Graphics&, juce::Rectangle<float>) const;
    void drawVU(juce::Graphics&, juce::Rectangle<float>) const;
    void drawMeters(juce::Graphics&, juce::Rectangle<float>) const;
    void drawSpectrum(juce::Graphics&, juce::Rectangle<float>) const;
    void drawWaveform(juce::Graphics&, juce::Rectangle<float>) const;
    void drawCompressionCurve(juce::Graphics&, juce::Rectangle<float>) const;
    void drawGRHistory(juce::Graphics&, juce::Rectangle<float>) const;
    void drawBottomTelemetry(juce::Graphics&, juce::Rectangle<float>) const;

    void storeA();
    void storeB();
    void restoreA();
    void restoreB();
    void resetToDefault();
    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BBTubeCompressorAudioProcessorEditor)
};
