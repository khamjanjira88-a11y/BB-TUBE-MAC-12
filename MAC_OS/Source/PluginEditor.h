#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

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

    juce::ComboBox styleBox, modeBox;
    juce::ToggleButton limiterButton, rmsButton, truePeakButton;

    juce::Slider input, threshold, ratio, attack, release, knee;
    juce::Slider tubeDrive, tubeBias, harmonics, mix, makeup;
    juce::Slider stereoLink, sidechainHPF, lookahead;
    juce::Slider limThreshold, ceiling, limRelease;

    std::unique_ptr<SliderAttachment> aInput, aThreshold, aRatio, aAttack, aRelease, aKnee;
    std::unique_ptr<SliderAttachment> aTubeDrive, aTubeBias, aHarmonics, aMix, aMakeup;
    std::unique_ptr<SliderAttachment> aStereoLink, aSidechainHPF, aLookahead;
    std::unique_ptr<SliderAttachment> aLimThreshold, aCeiling, aLimRelease;
    std::unique_ptr<ButtonAttachment> aLimiter, aRms, aTruePeak;
    std::unique_ptr<ComboAttachment> aStyle, aMode;

    void setupSlider(juce::Slider& slider, const juce::String& suffix, bool percent = false);
    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BBTubeCompressorAudioProcessorEditor)
};
