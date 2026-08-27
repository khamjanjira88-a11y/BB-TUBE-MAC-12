#include "PluginEditor.h"

namespace
{
class BBLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    BBLookAndFeel()
    {
        setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(0xff07090c));
        setColour(juce::Slider::thumbColourId, juce::Colour(0xffd8a44a));
        setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffd8a44a));
        setColour(juce::Slider::trackColourId, juce::Colour(0xff242a31));
        setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff11161d));
        setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff48525c));
        setColour(juce::TextButton::buttonColourId, juce::Colour(0xff11161d));
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h,
                          float pos, float startAngle, float endAngle,
                          juce::Slider&) override
    {
        const auto bounds = juce::Rectangle<float>((float)x, (float)y, (float)w, (float)h).reduced(9.0f);
        const auto centre = bounds.getCentre();
        const float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.38f;

        g.setColour(juce::Colour(0xff0e1217));
        g.fillEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);

        g.setColour(juce::Colour(0xff3d4650));
        g.drawEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f, 1.0f);

        const float angle = startAngle + pos * (endAngle - startAngle);
        const float pointer = radius - 5.0f;

        g.setColour(juce::Colour(0xffd8a44a));
        g.drawLine(centre.x, centre.y,
                   centre.x + std::cos(angle) * pointer,
                   centre.y + std::sin(angle) * pointer, 2.2f);

        g.setColour(juce::Colour(0xffedf2f6));
        g.fillEllipse(centre.x - 2.0f, centre.y - 2.0f, 4.0f, 4.0f);
    }
};

BBLookAndFeel bbLook;
}

BBTubeCompressorAudioProcessorEditor::BBTubeCompressorAudioProcessorEditor(
    BBTubeCompressorAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setLookAndFeel(&bbLook);
    setResizable(true, true);
    setResizeLimits(980, 620, 2600, 1500);
    setSize(1400, 860);

    setupSlider(input, "dB");
    setupSlider(threshold, "dB");
    setupSlider(ratio, ":1");
    setupSlider(attack, "ms");
    setupSlider(release, "ms");
    setupSlider(knee, "dB");
    setupSlider(tubeDrive, "dB");
    setupSlider(tubeBias, "");
    setupSlider(harmonics, "%", true);
    setupSlider(mix, "%", true);
    setupSlider(makeup, "dB");
    setupSlider(stereoLink, "%", true);
    setupSlider(sidechainHPF, "Hz");
    setupSlider(lookahead, "ms");
    setupSlider(limThreshold, "dB");
    setupSlider(ceiling, "dB");
    setupSlider(limRelease, "ms");

    auto& apvts = processor.parameters();
    aInput = std::make_unique<SliderAttachment>(apvts, "input", input);
    aThreshold = std::make_unique<SliderAttachment>(apvts, "threshold", threshold);
    aRatio = std::make_unique<SliderAttachment>(apvts, "ratio", ratio);
    aAttack = std::make_unique<SliderAttachment>(apvts, "attack", attack);
    aRelease = std::make_unique<SliderAttachment>(apvts, "release", release);
    aKnee = std::make_unique<SliderAttachment>(apvts, "knee", knee);
    aTubeDrive = std::make_unique<SliderAttachment>(apvts, "tubeDrive", tubeDrive);
    aTubeBias = std::make_unique<SliderAttachment>(apvts, "tubeBias", tubeBias);
    aHarmonics = std::make_unique<SliderAttachment>(apvts, "harmonics", harmonics);
    aMix = std::make_unique<SliderAttachment>(apvts, "mix", mix);
    aMakeup = std::make_unique<SliderAttachment>(apvts, "makeup", makeup);
    aStereoLink = std::make_unique<SliderAttachment>(apvts, "stereoLink", stereoLink);
    aSidechainHPF = std::make_unique<SliderAttachment>(apvts, "sidechainHPF", sidechainHPF);
    aLookahead = std::make_unique<SliderAttachment>(apvts, "lookahead", lookahead);
    aLimThreshold = std::make_unique<SliderAttachment>(apvts, "limThreshold", limThreshold);
    aCeiling = std::make_unique<SliderAttachment>(apvts, "ceiling", ceiling);
    aLimRelease = std::make_unique<SliderAttachment>(apvts, "limRelease", limRelease);

    limiterButton.setButtonText("LIMITER");
    rmsButton.setButtonText("RMS");
    truePeakButton.setButtonText("TRUE PEAK");
    addAndMakeVisible(limiterButton);
    addAndMakeVisible(rmsButton);
    addAndMakeVisible(truePeakButton);

    aLimiter = std::make_unique<ButtonAttachment>(apvts, "limiter", limiterButton);
    aRms = std::make_unique<ButtonAttachment>(apvts, "rms", rmsButton);
    aTruePeak = std::make_unique<ButtonAttachment>(apvts, "truePeak", truePeakButton);

    styleBox.addItemList({ "OPTO", "BUS", "FET" }, 1);
    modeBox.addItemList({ "STEREO", "MONO", "MID / SIDE", "DUAL MONO" }, 1);
    addAndMakeVisible(styleBox);
    addAndMakeVisible(modeBox);
    aStyle = std::make_unique<ComboAttachment>(apvts, "style", styleBox);
    aMode = std::make_unique<ComboAttachment>(apvts, "mode", modeBox);

    startTimerHz(30);
}

BBTubeCompressorAudioProcessorEditor::~BBTubeCompressorAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void BBTubeCompressorAudioProcessorEditor::setupSlider(
    juce::Slider& slider, const juce::String&, bool)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
    slider.setDoubleClickReturnValue(true, 0.0);
    addAndMakeVisible(slider);
}

void BBTubeCompressorAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff07090c));

    auto area = getLocalBounds().reduced(10);
    g.setColour(juce::Colour(0xff12171d));
    g.fillRoundedRectangle(area.toFloat(), 14.0f);

    g.setColour(juce::Colour(0xffcf9840));
    g.drawRoundedRectangle(area.toFloat(), 14.0f, 1.2f);

    // Header
    auto header = area.removeFromTop(68).reduced(12, 8);
    g.setColour(juce::Colour(0xff0c1015));
    g.fillRoundedRectangle(header.toFloat(), 9.0f);

    g.setColour(juce::Colour(0xffffc968));
    g.setFont(juce::FontOptions(25.0f).withStyle("bold"));
    g.drawText("BB TUBE", header.getX() + 16, header.getY() + 3, 190, 30, juce::Justification::left);

    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(11.0f));
    g.drawText("COMPRESSOR • OPTICAL • BUS • LIMITER", header.getX() + 18, header.getY() + 34, 300, 18, juce::Justification::left);

    // Main VU panel
    auto vu = area.removeFromTop(170).reduced(12);
    g.setColour(juce::Colour(0xffded8ce));
    g.fillRoundedRectangle(vu.toFloat(), 10.0f);

    auto meter = vu.reduced(16);
    g.setColour(juce::Colour(0xff2a241b));
    g.fillRoundedRectangle(meter.toFloat(), 8.0f);
    g.setColour(juce::Colour(0xffefa850));
    g.drawRoundedRectangle(meter.toFloat(), 8.0f, 1.5f);

    g.setColour(juce::Colour(0xffffd37f));
    g.setFont(juce::FontOptions(22.0f).withStyle("bold"));
    g.drawText("BB TUBE", meter.getX(), meter.getY() + 18, meter.getWidth(), 30, juce::Justification::centred);
    g.setFont(juce::FontOptions(10.0f));
    g.drawText("OPTICAL LEVELING / BUS CONTROL", meter.getX(), meter.getY() + 50, meter.getWidth(), 20, juce::Justification::centred);

    // VU needle
    const auto c = meter.getCentre().translated(0.0f, 26.0f);
    const float r = std::min((float) meter.getWidth(), (float) meter.getHeight()) * 0.35f;
    const float gr = juce::jlimit(0.0f, 24.0f, processor.getGainReductionDb());
    const float needle = juce::MathConstants<float>::pi * 0.78f
                       + juce::jlimit(0.0f, 24.0f, 24.0f - gr) / 24.0f
                         * (juce::MathConstants<float>::pi * 0.44f);
    g.setColour(juce::Colour(0xff14110e));
    g.drawLine(c.x, c.y, c.x + std::cos(needle) * r, c.y + std::sin(needle) * r, 3.0f);

    // Bottom analyzer area
    auto analyzer = area.removeFromBottom(190).reduced(12);
    g.setColour(juce::Colour(0xff0b1117));
    g.fillRoundedRectangle(analyzer.toFloat(), 10.0f);
    g.setColour(juce::Colour(0xff26313b));
    g.drawRoundedRectangle(analyzer.toFloat(), 10.0f, 1.0f);

    auto left = analyzer.removeFromLeft(analyzer.getWidth() * 0.55f);
    g.setColour(juce::Colour(0xff37a7ff));
    juce::Path path;
    const int samples = std::max(20, left.getWidth());
    for (int i = 0; i < samples; ++i)
    {
        const float t = (float) i / (float) (samples - 1);
        const float x = left.getX() + t * left.getWidth();
        const float y = left.getBottom() - 25.0f
                      - (0.45f + 0.5f * std::sin(t * 22.0f) + 0.18f * std::sin(t * 71.0f))
                        * left.getHeight() * 0.42f;
        if (i == 0) path.startNewSubPath(x, y);
        else path.lineTo(x, y);
    }
    g.strokePath(path, juce::PathStrokeType(1.4f));

    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(10.0f));
    g.drawText("REAL-TIME SPECTRUM / DYNAMICS", left.getX() + 10, left.getY() + 8, 260, 18, juce::Justification::left);
    g.drawText("IN  " + juce::String(processor.getInputPeakDb(), 1) + " dB",
               left.getRight() - 150, left.getY() + 8, 140, 18, juce::Justification::right);
    g.drawText("GR  " + juce::String(processor.getGainReductionDb(), 1) + " dB",
               left.getRight() - 150, left.getY() + 28, 140, 18, juce::Justification::right);

    auto info = analyzer;
    g.setColour(juce::Colour(0xffd8a44a));
    g.setFont(juce::FontOptions(12.0f).withStyle("bold"));
    g.drawText("BB TUBE COMPRESSOR", info.getX(), info.getY() + 12, info.getWidth(), 25, juce::Justification::centred);
    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(10.0f));
    g.drawText("Tube Character  •  Stereo Link  •  True Peak Limiter", info.getX(), info.getY() + 42, info.getWidth(), 20, juce::Justification::centred);
    g.drawText("Input / Output / Gain Reduction / Spectrum", info.getX(), info.getY() + 72, info.getWidth(), 20, juce::Justification::centred);
}

void BBTubeCompressorAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(22);
    area.removeFromTop(88);
    area.removeFromTop(185);

    auto row1 = area.removeFromTop(150).reduced(8);
    auto place = [&row1](juce::Slider& s)
    {
        s.setBounds(row1.removeFromLeft(92));
        row1.removeFromLeft(5);
    };

    place(input); place(threshold); place(ratio); place(attack); place(release);
    place(knee); place(tubeDrive); place(tubeBias); place(harmonics); place(mix);
    place(makeup); place(stereoLink);

    auto row2 = area.removeFromTop(150).reduced(8);
    auto p2 = [&row2](juce::Slider& s)
    {
        s.setBounds(row2.removeFromLeft(98));
        row2.removeFromLeft(7);
    };

    p2(sidechainHPF);
    p2(lookahead);
    p2(limThreshold);
    p2(ceiling);
    p2(limRelease);

    styleBox.setBounds(row2.removeFromLeft(120).withY(row2.getY() + 42).withHeight(28));
    modeBox.setBounds(row2.removeFromLeft(120).withY(row2.getY() + 42).withHeight(28));

    limiterButton.setBounds(row2.removeFromLeft(110).withY(row2.getY() + 18).withHeight(28));
    rmsButton.setBounds(row2.removeFromLeft(80).withY(row2.getY() + 18).withHeight(28));
    truePeakButton.setBounds(row2.withY(row2.getY() + 18).withHeight(28));
}
void BBTubeCompressorAudioProcessorEditor::timerCallback() { repaint(); }
