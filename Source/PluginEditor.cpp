#include "PluginEditor.h"

namespace
{
const juce::Colour kBg (0xff06080b);
const juce::Colour kPanel (0xff0f151b);
const juce::Colour kPanel2 (0xff131b22);
const juce::Colour kLine (0xff39444e);
const juce::Colour kGold (0xffdca64c);
const juce::Colour kGoldBright (0xffffcc70);
const juce::Colour kBlue (0xff33a8ff);
const juce::Colour kGreen (0xff47d879);
const juce::Colour kRed (0xffef4d42);

void setTextColour(juce::Graphics& g, float size, bool bold = false)
{
    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(size).withStyle(bold ? "bold" : "plain"));
}

void panel(juce::Graphics& g, juce::Rectangle<float> r)
{
    g.setColour(kPanel);
    g.fillRoundedRectangle(r, 8.0f);
    g.setColour(kLine);
    g.drawRoundedRectangle(r, 8.0f, 1.0f);
}
}

class BBTubeCompressorAudioProcessorEditor::BBLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    BBLookAndFeel()
    {
        setColour(juce::ResizableWindow::backgroundColourId, kBg);
        setColour(juce::Slider::thumbColourId, kGoldBright);
        setColour(juce::Slider::trackColourId, juce::Colour(0xff273039));
        setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff0e141a));
        setColour(juce::ComboBox::outlineColourId, kLine);
        setColour(juce::TextButton::buttonColourId, juce::Colour(0xff151c23));
        setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        setColour(juce::ToggleButton::textColourId, juce::Colours::white);
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h,
                          float pos, float startAngle, float endAngle,
                          juce::Slider&) override
    {
        const auto b = juce::Rectangle<float>((float)x, (float)y, (float)w, (float)h).reduced(7.0f);
        const auto c = b.getCentre();
        const float radius = juce::jmin(b.getWidth(), b.getHeight()) * 0.34f;

        g.setColour(juce::Colour(0xff090c10));
        g.fillEllipse(c.x - radius, c.y - radius, radius * 2.0f, radius * 2.0f);
        g.setColour(juce::Colour(0xff59636d));
        g.drawEllipse(c.x - radius, c.y - radius, radius * 2.0f, radius * 2.0f, 1.0f);

        const float angle = startAngle + pos * (endAngle - startAngle);
        g.setColour(kGoldBright);
        g.drawLine(c.x, c.y,
                   c.x + std::cos(angle) * (radius - 4.0f),
                   c.y + std::sin(angle) * (radius - 4.0f), 2.0f);

        g.setColour(juce::Colours::white);
        g.fillEllipse(c.x - 1.8f, c.y - 1.8f, 3.6f, 3.6f);
    }
};

BBTubeCompressorAudioProcessorEditor::BBTubeCompressorAudioProcessorEditor(
    BBTubeCompressorAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p), lookAndFeel(std::make_unique<BBLookAndFeel>())
{
    setLookAndFeel(lookAndFeel.get());
    setResizable(true, true);
    setResizeLimits(1280, 760, 3200, 1800);
    setSize(1720, 980);

    setupSlider(input, " dB");
    setupSlider(output, " dB");
    setupSlider(threshold, " dB");
    setupSlider(ratio, " :1");
    setupSlider(attack, " ms");
    setupSlider(release, " ms");
    setupSlider(knee, " dB");
    setupSlider(mix, " %");
    setupSlider(stereoLink, " %");
    setupSlider(makeup, " dB");
    setupSlider(tubeDrive, " dB");
    setupSlider(tubeBias, "");
    setupSlider(harmonics, " %");
    setupSlider(scHPF, " Hz");
    setupSlider(scLPF, " Hz");
    setupSlider(lookahead, " ms");
    setupSlider(limThreshold, " dB");
    setupSlider(ceiling, " dB");
    setupSlider(limRelease, " ms");

    auto& apvts = processor.parameters();

    addAttachment(input, "input", aInput);
    addAttachment(output, "output", aOutput);
    addAttachment(threshold, "threshold", aThreshold);
    addAttachment(ratio, "ratio", aRatio);
    addAttachment(attack, "attack", aAttack);
    addAttachment(release, "release", aRelease);
    addAttachment(knee, "knee", aKnee);
    addAttachment(mix, "mix", aMix);
    addAttachment(stereoLink, "stereoLink", aStereoLink);
    addAttachment(makeup, "makeup", aMakeup);
    addAttachment(tubeDrive, "tubeDrive", aTubeDrive);
    addAttachment(tubeBias, "tubeBias", aTubeBias);
    addAttachment(harmonics, "harmonics", aHarmonics);
    addAttachment(scHPF, "scHPF", aScHPF);
    addAttachment(scLPF, "scLPF", aScLPF);
    addAttachment(lookahead, "lookahead", aLookahead);
    addAttachment(limThreshold, "limThreshold", aLimThreshold);
    addAttachment(ceiling, "ceiling", aCeiling);
    addAttachment(limRelease, "limRelease", aLimRelease);

    limiterButton.setButtonText("LIMITER");
    truePeakButton.setButtonText("TRUE PEAK");
    rmsButton.setButtonText("RMS");
    autoMakeupButton.setButtonText("AUTO");
    warmthButton.setButtonText("WARM");

    addAndMakeVisible(limiterButton);
    addAndMakeVisible(truePeakButton);
    addAndMakeVisible(rmsButton);
    addAndMakeVisible(autoMakeupButton);
    addAndMakeVisible(warmthButton);

    aLimiter = std::make_unique<ButtonAttachment>(apvts, "limiter", limiterButton);
    aTruePeak = std::make_unique<ButtonAttachment>(apvts, "truePeak", truePeakButton);
    aRms = std::make_unique<ButtonAttachment>(apvts, "rms", rmsButton);
    aAutoMakeup = std::make_unique<ButtonAttachment>(apvts, "autoMakeup", autoMakeupButton);
    aWarmth = std::make_unique<ButtonAttachment>(apvts, "warmth", warmthButton);

    styleBox.addItemList({"OPTO", "VCA", "FET"}, 1);
    modeBox.addItemList({"STEREO", "MONO", "MID / SIDE", "DUAL MONO"}, 1);
    detectorBox.addItemList({"PEAK", "RMS"}, 1);
    thrustBox.addItemList({"CLEAN", "WARM", "HOT"}, 1);
    slopeBox.addItemList({"12 dB", "24 dB"}, 1);
    oversamplingBox.addItemList({"OFF", "2x", "4x", "8x"}, 1);

    addAndMakeVisible(styleBox);
    addAndMakeVisible(modeBox);
    addAndMakeVisible(detectorBox);
    addAndMakeVisible(thrustBox);
    addAndMakeVisible(slopeBox);
    addAndMakeVisible(oversamplingBox);

    aStyle = std::make_unique<ComboAttachment>(apvts, "style", styleBox);
    aMode = std::make_unique<ComboAttachment>(apvts, "mode", modeBox);
    aDetector = std::make_unique<ComboAttachment>(apvts, "detector", detectorBox);
    aThrust = std::make_unique<ComboAttachment>(apvts, "thrust", thrustBox);
    aSlope = std::make_unique<ComboAttachment>(apvts, "slope", slopeBox);
    aOversampling = std::make_unique<ComboAttachment>(apvts, "oversampling", oversamplingBox);

    const juce::StringArray presets {
        "Default", "Vocal Warm", "Vocal Aggressive", "Vocal Ballad",
        "Bass Warm", "Bass Punch", "Kick Drum", "Snare Glue",
        "Drum Bus Glue", "Drum Crush", "Mix Bus Glue", "Mix Bus Warm",
        "Master Gentle", "Master Punch", "Master Limiter", "Acoustic Guitar",
        "Piano", "Guitar Lead", "Synth Glue", "Vintage Vibe", "Heavy Tube"
    };
    presetBox.addItemList(presets, 1);
    presetBox.setSelectedId(1, juce::dontSendNotification);
    presetBox.onChange = [this] { processor.loadFactoryPreset(presetBox.getText()); };
    addAndMakeVisible(presetBox);

    for (auto* b : { &compareButton, &abButton, &saveButton, &resetButton })
        addAndMakeVisible(b);

    compareButton.onClick = [this]
    {
        if (!stateA) storeA();
        else restoreA();
    };
    abButton.onClick = [this]
    {
        if (usingB) restoreA();
        else restoreB();
    };
    saveButton.onClick = [this]
    {
        storeA();
        presetBox.setText("User Snapshot A", juce::dontSendNotification);
    };
    resetButton.onClick = [this] { resetToDefault(); };

    startTimerHz(30);
}

BBTubeCompressorAudioProcessorEditor::~BBTubeCompressorAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void BBTubeCompressorAudioProcessorEditor::setupSlider(juce::Slider& s, const juce::String& suffix)
{
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
    s.setDoubleClickReturnValue(true, 0.0);
    s.setTextValueSuffix(suffix);
    s.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    addAndMakeVisible(s);
}

void BBTubeCompressorAudioProcessorEditor::addAttachment(
    juce::Slider& slider, const char* id,
    std::unique_ptr<SliderAttachment>& attachment)
{
    attachment = std::make_unique<SliderAttachment>(processor.parameters(), id, slider);
}

void BBTubeCompressorAudioProcessorEditor::drawPanel(juce::Graphics& g, juce::Rectangle<float> r) const
{
    panel(g, r);
}

void BBTubeCompressorAudioProcessorEditor::addSectionLabel(
    juce::Graphics& g, const juce::Rectangle<int>& r, const juce::String& title) const
{
    g.setColour(kGoldBright);
    g.setFont(juce::FontOptions(10.0f).withStyle("bold"));
    g.drawText(title, r, juce::Justification::centredTop);
}

void BBTubeCompressorAudioProcessorEditor::drawVU(
    juce::Graphics& g, juce::Rectangle<float> r) const
{
    g.setColour(juce::Colour(0xff29231b));
    g.fillRoundedRectangle(r, 8.0f);
    g.setColour(kGold);
    g.drawRoundedRectangle(r, 8.0f, 1.4f);

    auto inner = r.reduced(12.0f);
    g.setColour(juce::Colour(0xff090b0e));
    g.fillRoundedRectangle(inner, 5.0f);

    g.setColour(kGoldBright);
    g.setFont(juce::FontOptions(23.0f).withStyle("bold"));
    g.drawText("BB TUBE", inner.getX(), inner.getY() + 18.0f,
               inner.getWidth(), 28.0f, juce::Justification::centred);

    g.setColour(juce::Colour(0xffd9b170));
    g.setFont(juce::FontOptions(8.5f));
    g.drawText("OPTICAL TUBE LEVELING AMPLIFIER", inner.getX(), inner.getY() + 48.0f,
               inner.getWidth(), 18.0f, juce::Justification::centred);

    const auto center = juce::Point<float>(inner.getCentreX(), inner.getBottom() - 18.0f);
    const float radius = juce::jmin(inner.getWidth(), inner.getHeight()) * 0.40f;
    const float gr = juce::jlimit(0.0f, 24.0f, processor.gainReductionDb());
    const float t = juce::jlimit(0.0f, 1.0f, 1.0f - gr / 24.0f);
    const float start = juce::MathConstants<float>::pi * 1.18f;
    const float end = juce::MathConstants<float>::pi * 1.82f;
    const float angle = start + t * (end - start);

    g.setColour(juce::Colour(0xff776b5c));
    g.drawArc(center.x - radius, center.y - radius,
              radius * 2.0f, radius * 2.0f,
              start, end, 2.0f);

    g.setColour(kGoldBright);
    g.drawLine(center.x, center.y,
               center.x + std::cos(angle) * radius * 0.92f,
               center.y + std::sin(angle) * radius * 0.92f,
               2.4f);

    g.setColour(kGoldBright);
    g.setFont(juce::FontOptions(9.0f).withStyle("bold"));
    g.drawText("GR  " + juce::String(processor.gainReductionDb(), 1) + " dB",
               inner.getX() + 8.0f, inner.getBottom() - 22.0f,
               140.0f, 16.0f, juce::Justification::left);
}

void BBTubeCompressorAudioProcessorEditor::drawMeters(
    juce::Graphics& g, juce::Rectangle<float> r) const
{
    panel(g, r);
    const float gap = 13.0f;
    const float meterW = 22.0f;
    auto area = r.reduced(12.0f);
    auto one = area.removeFromLeft(meterW);
    auto two = area.removeFromLeft(meterW);
    area.removeFromLeft(gap);
    auto three = area.removeFromLeft(meterW);

    const auto draw = [&](juce::Rectangle<float> m, float db, float floor, const juce::String& label)
    {
        g.setColour(juce::Colour(0xff05080b));
        g.fillRoundedRectangle(m, 4.0f);
        const float norm = juce::jmap(juce::jlimit(floor, 0.0f, db), floor, 0.0f, 0.0f, 1.0f);
        const float h = m.getHeight() * norm;
        g.setColour(db > -3.0f ? kRed : (db > -12.0f ? kGold : kGreen));
        g.fillRoundedRectangle(m.withTop(m.getBottom() - h), 4.0f);
        g.setColour(juce::Colours::white);
        g.setFont(juce::FontOptions(8.0f).withStyle("bold"));
        g.drawText(label, m.getX() - 5.0f, m.getBottom() + 3.0f,
                   m.getWidth() + 10.0f, 14.0f, juce::Justification::centred);
    };

    draw(one, processor.inputPeakDb(), -60.0f, "IN");
    draw(two, -processor.gainReductionDb(), -24.0f, "GR");
    draw(three, processor.outputPeakDb(), -60.0f, "OUT");

    setTextColour(g, 9.0f);
    g.drawText("IN  " + juce::String(processor.inputPeakDb(), 1) + " dB",
               area.getX(), area.getY(), 110, 16, juce::Justification::left);
    g.drawText("OUT " + juce::String(processor.outputPeakDb(), 1) + " dB",
               area.getX(), area.getY() + 18, 110, 16, juce::Justification::left);
    g.drawText("TP  " + juce::String(processor.samplePeakDb(), 1) + " dBTP",
               area.getX(), area.getY() + 36, 110, 16, juce::Justification::left);
}

void BBTubeCompressorAudioProcessorEditor::drawSpectrum(
    juce::Graphics& g, juce::Rectangle<float> r) const
{
    panel(g, r);
    addSectionLabel(g, r.toNearestInt(), "REAL-TIME SPECTRUM");
    auto graph = r.reduced(8.0f);
    graph.removeFromTop(21.0f);

    for (int i = 1; i < 6; ++i)
    {
        const float y = graph.getY() + graph.getHeight() * (float)i / 6.0f;
        g.setColour(juce::Colour(0xff202931));
        g.drawHorizontalLine((int)y, graph.getX(), graph.getRight());
    }

    juce::Path p;
    const int w = juce::jmax(1, (int)graph.getWidth());
    for (int x = 0; x < w; ++x)
    {
        const float t = (float)x / (float)juce::jmax(1, w - 1);
        const int bin = juce::jlimit(1, BBTubeCompressorAudioProcessor::kSpectrumBins - 1,
                                     (int)std::pow(t, 1.7f) * (BBTubeCompressorAudioProcessor::kSpectrumBins - 1));
        const float db = juce::jlimit(-100.0f, 0.0f, spectrum[(size_t)bin]);
        const float n = juce::jmap(db, -100.0f, 0.0f, 1.0f, 0.0f);
        const float xx = graph.getX() + t * graph.getWidth();
        const float yy = graph.getY() + n * graph.getHeight();
        if (x == 0) p.startNewSubPath(xx, yy); else p.lineTo(xx, yy);
    }
    g.setColour(kBlue);
    g.strokePath(p, juce::PathStrokeType(1.6f));

    setTextColour(g, 8.0f);
    g.drawText("20 Hz", graph.getX(), graph.getBottom() - 12, 55, 12, juce::Justification::left);
    g.drawText("20 kHz", graph.getRight() - 55, graph.getBottom() - 12, 55, 12, juce::Justification::right);
}

void BBTubeCompressorAudioProcessorEditor::drawWaveform(
    juce::Graphics& g, juce::Rectangle<float> r) const
{
    panel(g, r);
    addSectionLabel(g, r.toNearestInt(), "WAVEFORM");
    auto graph = r.reduced(8.0f);
    graph.removeFromTop(21.0f);

    juce::Path p;
    const int w = juce::jmax(1, (int)graph.getWidth());
    for (int x = 0; x < w; ++x)
    {
        const float t = (float)x / (float)juce::jmax(1, w - 1);
        const int index = juce::jlimit(0, BBTubeCompressorAudioProcessor::kWaveformSize - 1,
                                       (int)(t * (BBTubeCompressorAudioProcessor::kWaveformSize - 1)));
        const float s = juce::jlimit(-1.0f, 1.0f, waveform[(size_t)index]);
        const float xx = graph.getX() + t * graph.getWidth();
        const float yy = graph.getCentreY() - s * graph.getHeight() * 0.42f;
        if (x == 0) p.startNewSubPath(xx, yy); else p.lineTo(xx, yy);
    }

    g.setColour(kBlue);
    g.strokePath(p, juce::PathStrokeType(1.3f));
}

void BBTubeCompressorAudioProcessorEditor::drawCompressionCurve(
    juce::Graphics& g, juce::Rectangle<float> r) const
{
    panel(g, r);
    addSectionLabel(g, r.toNearestInt(), "COMPRESSION CURVE");
    auto graph = r.reduced(8.0f);
    graph.removeFromTop(21.0f);

    g.setColour(juce::Colour(0xff2b343c));
    g.drawLine(graph.getX(), graph.getBottom(), graph.getRight(), graph.getY(), 1.0f);

    const float threshold = processor.parameters().getRawParameterValue("threshold")->load();
    const float ratio = processor.parameters().getRawParameterValue("ratio")->load();

    juce::Path curve;
    for (int i = 0; i <= 100; ++i)
    {
        const float inDb = -60.0f + 0.6f * (float)i;
        const float outDb = inDb <= threshold
            ? inDb
            : threshold + (inDb - threshold) / ratio;

        const float x = juce::jmap(inDb, -60.0f, 0.0f, graph.getX(), graph.getRight());
        const float y = juce::jmap(outDb, -60.0f, 0.0f, graph.getBottom(), graph.getY());
        if (i == 0) curve.startNewSubPath(x, y); else curve.lineTo(x, y);
    }

    g.setColour(kGoldBright);
    g.strokePath(curve, juce::PathStrokeType(1.8f));

    g.setColour(kRed);
    const float ceiling = juce::jmap(juce::jlimit(-60.0f, 0.0f, processor.parameters().getRawParameterValue("ceiling")->load()),
                                     -60.0f, 0.0f, graph.getBottom(), graph.getY());
    g.drawHorizontalLine((int)ceiling, graph.getX(), graph.getRight());
}

void BBTubeCompressorAudioProcessorEditor::drawGRHistory(
    juce::Graphics& g, juce::Rectangle<float> r) const
{
    panel(g, r);
    addSectionLabel(g, r.toNearestInt(), "GAIN REDUCTION HISTORY");
    auto graph = r.reduced(8.0f);
    graph.removeFromTop(21.0f);

    juce::Path p;
    const int w = juce::jmax(1, (int)graph.getWidth());
    for (int x = 0; x < w; ++x)
    {
        const float t = (float)x / (float)juce::jmax(1, w - 1);
        const int index = juce::jlimit(0, BBTubeCompressorAudioProcessor::kHistorySize - 1,
                                       (int)(t * (BBTubeCompressorAudioProcessor::kHistorySize - 1)));
        const float gr = juce::jlimit(0.0f, 24.0f, gainHistory[(size_t)index]);
        const float yy = juce::jmap(gr, 0.0f, 24.0f, graph.getY(), graph.getBottom());
        const float xx = graph.getX() + t * graph.getWidth();
        if (x == 0) p.startNewSubPath(xx, yy); else p.lineTo(xx, yy);
    }

    g.setColour(kRed);
    g.strokePath(p, juce::PathStrokeType(1.5f));
}

void BBTubeCompressorAudioProcessorEditor::drawBottomTelemetry(
    juce::Graphics& g, juce::Rectangle<float> r) const
{
    panel(g, r);
    auto col = r.reduced(10.0f);

    g.setColour(kGoldBright);
    g.setFont(juce::FontOptions(10.0f).withStyle("bold"));
    g.drawText("LOUDNESS", col.removeFromLeft(120.0f), juce::Justification::centred);

    const auto loudArea = col.removeFromLeft(150.0f);
    g.setColour(kGreen);
    g.setFont(juce::FontOptions(16.0f).withStyle("bold"));
    g.drawText(juce::String(processor.shortTermLufs(), 1) + " LUFS",
               loudArea.getX(), loudArea.getY() + 18.0f,
               loudArea.getWidth(), 22.0f, juce::Justification::centred);
    setTextColour(g, 9.0f);
    g.drawText("INT " + juce::String(processor.integratedLufs(), 1),
               loudArea.getX(), loudArea.getY() + 45.0f,
               loudArea.getWidth(), 16.0f, juce::Justification::centred);
    g.drawText("LRA " + juce::String(processor.loudnessRange(), 1),
               loudArea.getX(), loudArea.getY() + 64.0f,
               loudArea.getWidth(), 16.0f, juce::Justification::centred);

    const auto corrArea = col.removeFromLeft(160.0f);
    g.setColour(kGoldBright);
    g.setFont(juce::FontOptions(10.0f).withStyle("bold"));
    g.drawText("CORRELATION / BALANCE", corrArea.toNearestInt(), juce::Justification::centredTop);
    g.setColour(juce::Colour(0xff242d34));
    g.fillRoundedRectangle(corrArea.getX() + 12.0f, corrArea.getY() + 25.0f,
                           corrArea.getWidth() - 24.0f, 8.0f, 4.0f);
    const float norm = juce::jmap(processor.correlation(), -1.0f, 1.0f, 0.0f, 1.0f);
    g.setColour(kGreen);
    g.fillRoundedRectangle(corrArea.getX() + 12.0f, corrArea.getY() + 25.0f,
                           (corrArea.getWidth() - 24.0f) * norm, 8.0f, 4.0f);
    setTextColour(g, 9.0f);
    g.drawText("Corr " + juce::String(processor.correlation(), 2),
               corrArea.getX(), corrArea.getY() + 44.0f,
               corrArea.getWidth(), 16.0f, juce::Justification::centred);
    g.drawText("Bal " + juce::String(processor.balanceDb(), 1) + " dB",
               corrArea.getX(), corrArea.getY() + 63.0f,
               corrArea.getWidth(), 16.0f, juce::Justification::centred);

    const auto info = col;
    setTextColour(g, 9.0f);
    g.drawText("BB TUBE COMPRESSOR • Tube Character • Stereo Link • Limiter",
               info.toNearestInt(), juce::Justification::centred);
    g.drawText("Universal 2 macOS • Windows x64 • VST3 • AU",
               info.withY(info.getY() + 22.0f).toNearestInt(), juce::Justification::centred);
}

void BBTubeCompressorAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(kBg);

    auto outer = getLocalBounds().reduced(8).toFloat();
    g.setColour(kPanel2);
    g.fillRoundedRectangle(outer, 12.0f);
    g.setColour(kGold);
    g.drawRoundedRectangle(outer, 12.0f, 1.2f);

    auto content = outer.reduced(8.0f);

    // Header.
    auto header = content.removeFromTop(52.0f);
    panel(g, header);
    g.setColour(kGoldBright);
    g.setFont(juce::FontOptions(22.0f).withStyle("bold"));
    g.drawText("BB TUBE", header.getX() + 12.0f, header.getY() + 4.0f,
               160.0f, 25.0f, juce::Justification::left);
    setTextColour(g, 8.5f);
    g.drawText("COMPRESSOR • OPTICAL • BUS • LIMITER • COMMERCIAL EDITION",
               header.getX() + 14.0f, header.getY() + 29.0f,
               390.0f, 16.0f, juce::Justification::left);

    // Feature list left, top input, VU, output and meters right.
    auto hardware = content.removeFromTop(182.0f);
    const float leftW = 248.0f;
    const float rightW = 160.0f;
    auto left = hardware.removeFromLeft(leftW).reduced(4.0f);
    auto meters = hardware.removeFromRight(rightW).reduced(4.0f);
    auto center = hardware.reduced(4.0f);

    panel(g, left);
    g.setColour(kGoldBright);
    g.setFont(juce::FontOptions(10.0f).withStyle("bold"));
    g.drawText("FEATURES", left.getX() + 10.0f, left.getY() + 8.0f,
               left.getWidth() - 20.0f, 16.0f, juce::Justification::left);
    const juce::StringArray features {
        "Optical tube leveling",
        "Bus compression topology",
        "Tube saturation + harmonics",
        "Peak / RMS detection",
        "Soft Knee + variable release",
        "Sidechain HPF / LPF",
        "Lookahead limiter",
        "Stereo Link / M-S / Dual Mono",
        "Real-time spectrum + waveform",
        "Factory presets for mixing"
    };
    setTextColour(g, 8.2f);
    float yy = left.getY() + 28.0f;
    for (const auto& f : features)
    {
        g.setColour(kGold);
        g.fillEllipse(left.getX() + 11.0f, yy + 4.0f, 5.0f, 5.0f);
        g.setColour(juce::Colours::white);
        g.drawText(f, left.getX() + 23.0f, yy, left.getWidth() - 30.0f, 14.0f,
                   juce::Justification::left);
        yy += 14.5f;
    }

    panel(g, center);
    auto cArea = center.reduced(8.0f);
    const float dialW = cArea.getWidth() * 0.22f;
    auto inArea = cArea.removeFromLeft(dialW);
    auto outArea = cArea.removeFromRight(dialW);
    drawDialGroup:
    ;
    addSectionLabel(g, inArea.toNearestInt(), "INPUT");
    addSectionLabel(g, outArea.toNearestInt(), "OUTPUT");
    drawVU(g, cArea);

    drawMeters(g, meters);

    // Main compressor strip.
    auto controlStrip = content.removeFromTop(176.0f).reduced(4.0f);
    panel(g, controlStrip);

    auto row = controlStrip.reduced(6.0f);
    const float cellW = row.getWidth() / 10.0f;
    auto cell = [&](int i) { return juce::Rectangle<float>(row.getX() + cellW * i, row.getY(),
                                                             cellW, row.getHeight()).reduced(3.0f); };

    const auto labels = std::array<const char*, 8>{
        "THRESHOLD", "RATIO", "ATTACK", "RELEASE", "KNEE", "MIX", "LINK", "MAKEUP"};
    juce::Slider* sliders[] = { &threshold, &ratio, &attack, &release, &knee, &mix, &stereoLink, &makeup };
    for (int i = 0; i < 8; ++i)
    {
        addSectionLabel(g, cell(i).toNearestInt(), labels[(size_t)i]);
    }

    addSectionLabel(g, cell(8).toNearestInt(), "DETECTOR");
    addSectionLabel(g, cell(9).toNearestInt(), "STYLE / MODE");

    // Lower three sections.
    auto lower = content.removeFromTop(186.0f).reduced(4.0f);
    const float third = lower.getWidth() / 3.0f;
    auto tube = lower.removeFromLeft(third - 3.0f).reduced(3.0f);
    auto sc = lower.removeFromLeft(third - 3.0f).reduced(3.0f);
    auto lim = lower.reduced(3.0f);

    panel(g, tube);
    addSectionLabel(g, tube.toNearestInt(), "TUBE CHARACTER");
    panel(g, sc);
    addSectionLabel(g, sc.toNearestInt(), "SIDECHAIN / FILTER");
    panel(g, lim);
    addSectionLabel(g, lim.toNearestInt(), "LIMITER");

    // Bottom analyzer row.
    auto analyzers = content.removeFromTop(214.0f).reduced(4.0f);
    const float aw = analyzers.getWidth() / 4.0f;
    drawSpectrum(g, analyzers.removeFromLeft(aw - 3.0f).reduced(3.0f));
    drawWaveform(g, analyzers.removeFromLeft(aw - 3.0f).reduced(3.0f));
    drawCompressionCurve(g, analyzers.removeFromLeft(aw - 3.0f).reduced(3.0f));
    drawGRHistory(g, analyzers.reduced(3.0f));

    // Footer telemetry.
    drawBottomTelemetry(g, content.removeFromBottom(92.0f).reduced(4.0f));
}

void BBTubeCompressorAudioProcessorEditor::resized()
{
    const int w = getWidth();
    const int h = getHeight();

    // Header controls.
    presetBox.setBounds(w - 610, 17, 260, 28);
    compareButton.setBounds(w - 338, 17, 86, 28);
    abButton.setBounds(w - 244, 17, 64, 28);
    saveButton.setBounds(w - 172, 17, 62, 28);
    resetButton.setBounds(w - 102, 17, 78, 28);

    // Hardware top dials.
    input.setBounds(286, 88, 130, 125);
    output.setBounds(w - 430, 88, 130, 125);

    // Main control row.
    const int y1 = 252;
    const int height1 = 142;
    const int startX = 278;
    const int avail = w - 560;
    const int bw = juce::jmax(82, avail / 10);

    juce::Slider* mainSliders[] = {
        &threshold, &ratio, &attack, &release, &knee, &mix, &stereoLink, &makeup
    };
    for (int i = 0; i < 8; ++i)
        mainSliders[i]->setBounds(startX + i * bw, y1, bw - 4, height1);

    detectorBox.setBounds(startX + 8 * bw + 6, y1 + 26, bw - 12, 28);
    thrustBox.setBounds(startX + 8 * bw + 6, y1 + 62, bw - 12, 28);
    styleBox.setBounds(startX + 9 * bw + 6, y1 + 26, bw - 12, 28);
    modeBox.setBounds(startX + 9 * bw + 6, y1 + 62, bw - 12, 28);

    // Lower panels controls.
    const int y2 = 438;
    const int dial = 96;
    tubeDrive.setBounds(290, y2 + 22, dial, 125);
    tubeBias.setBounds(390, y2 + 22, dial, 125);
    harmonics.setBounds(490, y2 + 22, dial, 125);

    scHPF.setBounds(650, y2 + 22, dial, 125);
    scLPF.setBounds(750, y2 + 22, dial, 125);
    lookahead.setBounds(850, y2 + 22, dial, 125);

    limThreshold.setBounds(w - 670, y2 + 22, dial, 125);
    ceiling.setBounds(w - 570, y2 + 22, dial, 125);
    limRelease.setBounds(w - 470, y2 + 22, dial, 125);

    limiterButton.setBounds(w - 360, y2 + 32, 90, 28);
    truePeakButton.setBounds(w - 360, y2 + 68, 90, 28);
    rmsButton.setBounds(w - 260, y2 + 32, 76, 28);
    autoMakeupButton.setBounds(w - 260, y2 + 68, 76, 28);
    warmthButton.setBounds(w - 174, y2 + 32, 76, 28);
    oversamplingBox.setBounds(w - 174, y2 + 68, 76, 28);
    slopeBox.setBounds(w - 360, y2 + 104, 140, 26);
}

void BBTubeCompressorAudioProcessorEditor::storeA()
{
    stateA = processor.parameters().copyState().createXml();
    usingB = false;
}

void BBTubeCompressorAudioProcessorEditor::storeB()
{
    stateB = processor.parameters().copyState().createXml();
    usingB = true;
}

void BBTubeCompressorAudioProcessorEditor::restoreA()
{
    if (!stateA)
        return;

    processor.parameters().replaceState(juce::ValueTree::fromXml(*stateA));
    usingB = false;
}

void BBTubeCompressorAudioProcessorEditor::restoreB()
{
    if (!stateB)
        storeB();
    else
        processor.parameters().replaceState(juce::ValueTree::fromXml(*stateB));

    usingB = true;
}

void BBTubeCompressorAudioProcessorEditor::resetToDefault()
{
    processor.loadFactoryPreset("Default");
    presetBox.setSelectedId(1, juce::dontSendNotification);
}

void BBTubeCompressorAudioProcessorEditor::timerCallback()
{
    processor.copySpectrum(spectrum);
    processor.copyWaveform(waveform);
    processor.copyGainHistory(gainHistory);
    repaint();
}
