#include "PluginEditor.h"

const juce::Colour ReverbAudioProcessorEditor::bgTop(0xFF0C0E16);
const juce::Colour ReverbAudioProcessorEditor::bgBottom(0xFF181D2E);
const juce::Colour ReverbAudioProcessorEditor::cyan(0xFF45E3FF);
const juce::Colour ReverbAudioProcessorEditor::violet(0xFF9D6BFF);
const juce::Colour ReverbAudioProcessorEditor::magenta(0xFFFF5FA2);

void NeonKnob::paint(juce::Graphics& g)
{
    const auto area = getLocalBounds().toFloat();
    const float d = juce::jmin(area.getWidth(), area.getHeight() - 14.0f);
    const auto arc = area.withSizeKeepingCentre(d, d);

    const float cx = arc.getCentreX(), cy = arc.getCentreY();
    const float r = d * 0.5f;
    const float thickness = d * 0.095f;

    const float startAngle = juce::MathConstants<float>::pi * 1.25f;
    const float sweep = juce::MathConstants<float>::pi * 1.5f;
    const float valueAngle = startAngle + sweep * (float)valueToProportionOfLength(getValue());

    juce::Path trackPath;
    trackPath.addArc(arc.getX() + thickness, arc.getY() + thickness,
                     d - 2.0f * thickness, d - 2.0f * thickness,
                     startAngle, startAngle + sweep, true);
    g.setColour(juce::Colour(0xFF2A3148));
    g.strokePath(trackPath, juce::PathStrokeType(thickness, juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));

    juce::Path valuePath;
    valuePath.addArc(arc.getX() + thickness, arc.getY() + thickness,
                     d - 2.0f * thickness, d - 2.0f * thickness,
                     startAngle, valueAngle, true);

    g.setColour(juce::Colour(0xFF45E3FF).withAlpha(0.09f));
    g.strokePath(valuePath, juce::PathStrokeType(thickness * 4.2f, juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));
    g.setColour(juce::Colour(0xFF45E3FF).withAlpha(0.18f));
    g.strokePath(valuePath, juce::PathStrokeType(thickness * 2.6f, juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));

    juce::ColourGradient gradient(juce::Colour(0xFF45E3FF), cx - r, cy,
                                  juce::Colour(0xFF9D6BFF), cx + r, cy, false);
    g.setGradientFill(gradient);
    g.strokePath(valuePath, juce::PathStrokeType(thickness, juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));

    const float bodyR = r * 0.58f;
    const juce::Rectangle<float> body(cx - bodyR, cy - bodyR, bodyR * 2.0f, bodyR * 2.0f);

    g.setColour(juce::Colour(0xFF0B0E17));
    g.fillEllipse(body);

    juce::ColourGradient bodyGrad(juce::Colour(0xFF39415C), cx - bodyR * 0.4f, cy - bodyR * 0.4f,
                                  juce::Colour(0xFF141826), cx + bodyR * 0.4f, cy + bodyR * 0.4f, true);
    g.setGradientFill(bodyGrad);
    g.fillEllipse(body);

    g.setColour(juce::Colour(0xFF45E3FF).withAlpha(0.10f));
    g.drawEllipse(body, 1.0f);

    const float indR = r * 0.66f;
    const float ix = cx + std::cos(valueAngle) * indR;
    const float iy = cy + std::sin(valueAngle) * indR;
    g.setColour(juce::Colours::white.withAlpha(0.92f));
    g.fillEllipse(ix - 2.4f, iy - 2.4f, 4.8f, 4.8f);

    const float capR = r * 0.16f;
    g.setColour(juce::Colour(0xFF05060A));
    g.fillEllipse(cx - capR, cy - capR, capR * 2.0f, capR * 2.0f);

    juce::String display = getValueSuffix() == "%"
                               ? juce::String((int)(getValue() * 100.0)) + " %"
                               : juce::String(getValue(), 1) + " " + getValueSuffix();
    g.setFont(juce::FontOptions(10.5f));
    g.setColour(juce::Colours::white.withAlpha(0.55f));
    g.drawText(display, 0, (int)(area.getHeight() - 13.0f),
               (int)area.getWidth(), 12, juce::Justification::centred);
}

const juce::Colour PresetButton::accent(0xFF45E3FF);
const juce::Colour PresetButton::accentBright(0xFFBDF4FF);
const juce::Colour PresetButton::bgNormal(0x14FFFFFF);
const juce::Colour PresetButton::bgHover(0x24FFFFFF);
const juce::Colour PresetButton::bgSelected(0x3337E0FF);

void PresetButton::paintButton(juce::Graphics& g, bool highlighted, bool down)
{
    const auto r = getLocalBounds().toFloat();
    const bool hover = highlighted || down;

    g.setColour(selected ? bgSelected : (hover ? bgHover : bgNormal));
    g.fillRoundedRectangle(r, r.getHeight() * 0.5f);

    if (selected)
    {
        g.setColour(accent.withAlpha(0.55f));
        g.drawRoundedRectangle(r.reduced(1.0f), r.getHeight() * 0.5f, 1.5f);
    }

    g.setColour(selected ? accentBright
                         : (hover ? juce::Colours::white.withAlpha(0.95f)
                                  : juce::Colours::white.withAlpha(0.60f)));
    g.setFont(juce::FontOptions(r.getHeight() * 0.38f).withStyle(juce::Font::bold));
    g.drawText(getName(), r, juce::Justification::centred);
}

void PowerButton::paintButton(juce::Graphics& g, bool, bool)
{
    const auto r = getLocalBounds().toFloat();
    const bool on = getToggleState();

    const float h = r.getHeight() * 0.52f;
    const auto track = r.withSizeKeepingCentre(r.getWidth() * 0.50f, h);

    g.setColour(on ? juce::Colour(0xFF45E3FF).withAlpha(0.30f)
                   : juce::Colour(0x22FFFFFF));
    g.fillRoundedRectangle(track, h * 0.5f);

    const float dotR = h * 0.30f;
    const float dotX = on ? track.getRight() - h * 0.50f
                          : track.getX() + h * 0.50f;
    g.setColour(on ? juce::Colour(0xFF45E3FF)
                   : juce::Colours::white.withAlpha(0.35f));
    g.fillEllipse(dotX - dotR, track.getCentreY() - dotR, dotR * 2.0f, dotR * 2.0f);
}

static const float presets[6][8] =
{
    { 0.75f, 3.2f, 0.45f, 0.65f, 35.0f, 0.35f, 80.0f, 9000.0f },
    { 0.60f, 1.8f, 0.40f, 0.60f, 25.0f, 0.28f, 100.0f, 11000.0f },
    { 0.50f, 2.4f, 0.35f, 0.70f, 20.0f, 0.30f, 150.0f, 14000.0f },
    { 0.35f, 0.8f, 0.55f, 0.50f, 10.0f, 0.25f, 120.0f, 12000.0f },
    { 0.95f, 6.5f, 0.60f, 0.75f, 45.0f, 0.40f, 60.0f, 7500.0f },
    { 0.85f, 5.0f, 0.70f, 0.90f, 40.0f, 0.38f, 200.0f, 6000.0f }
};

ReverbAudioProcessorEditor::ReverbAudioProcessorEditor(ReverbAudioProcessor& p)
    : juce::AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(860, 520);
    setResizeLimits(780, 470, 1120, 720);

    titleLabel.setText("AURORA", juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(26.0f).withStyle(juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(titleLabel);

    subtitleLabel.setText("REVERB  ·  STUDIO SPACE", juce::dontSendNotification);
    subtitleLabel.setFont(juce::FontOptions(10.0f).withStyle(juce::Font::bold));
    subtitleLabel.setColour(juce::Label::textColourId, cyan.withAlpha(0.9f));
    addAndMakeVisible(subtitleLabel);

    versionLabel.setText("v1.0.0   VST3 / AU / CLAP", juce::dontSendNotification);
    versionLabel.setFont(juce::FontOptions(10.0f));
    versionLabel.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.35f));
    addAndMakeVisible(versionLabel);

    bypassButton.onClick = [this]
    {
        audioProcessor.getReverbProcessor()->setBypassed(bypassButton.getToggleState());
    };
    addAndMakeVisible(bypassButton);

    const char* presetNames[] = { "HALL", "CHAMBER", "PLATE", "ROOM", "CATHEDRAL", "SHIMMER" };
    for (int i = 0; i < 6; ++i)
    {
        auto* b = presetButtons.add(new PresetButton(presetNames[i]));
        b->onClick = [this, i] { applyPreset(i); };
        addAndMakeVisible(b);
    }

    const char* inputNames[]  = { "PRE-DELAY", "LOW CUT", "HIGH CUT" };
    const char* spaceNames[]  = { "SIZE", "DECAY", "DAMPING" };
    const char* outputNames[] = { "WIDTH", "MIX" };

    for (int i = 0; i < 3; ++i)
    {
        setupKnob(inputKnobs[i], inputNames[i]);
        setupKnob(spaceKnobs[i], spaceNames[i]);
    }
    for (int i = 0; i < 2; ++i)
        setupKnob(outputKnobs[i], outputNames[i]);

    inputKnobs[0].knob.setNormalisableRange(juce::NormalisableRange<double>(0.0, 250.0, 0.5));
    inputKnobs[0].knob.setValueSuffix("ms");
    inputKnobs[1].knob.setNormalisableRange(juce::NormalisableRange<double>(20.0, 500.0, 1.0, 0.6));
    inputKnobs[1].knob.setValueSuffix("Hz");
    inputKnobs[2].knob.setNormalisableRange(juce::NormalisableRange<double>(1000.0, 20000.0, 1.0, 0.5));
    inputKnobs[2].knob.setValueSuffix("Hz");

    spaceKnobs[0].knob.setNormalisableRange(juce::NormalisableRange<double>(0.0, 1.0, 0.001));
    spaceKnobs[0].knob.setValueSuffix("%");
    spaceKnobs[1].knob.setNormalisableRange(juce::NormalisableRange<double>(0.1, 10.0, 0.01, 0.5));
    spaceKnobs[1].knob.setValueSuffix("s");
    spaceKnobs[2].knob.setNormalisableRange(juce::NormalisableRange<double>(0.0, 1.0, 0.001));
    spaceKnobs[2].knob.setValueSuffix("%");

    outputKnobs[0].knob.setNormalisableRange(juce::NormalisableRange<double>(0.0, 1.0, 0.001));
    outputKnobs[0].knob.setValueSuffix("%");
    outputKnobs[1].knob.setNormalisableRange(juce::NormalisableRange<double>(0.0, 1.0, 0.001));
    outputKnobs[1].knob.setValueSuffix("%");

    const char* paramIds[] = { "predelay", "loCut", "hiCut",
                               "roomSize", "decay", "damping",
                               "width", "mix" };
    NeonKnob* knobs[] = { &inputKnobs[0].knob, &inputKnobs[1].knob, &inputKnobs[2].knob,
                          &spaceKnobs[0].knob, &spaceKnobs[1].knob, &spaceKnobs[2].knob,
                          &outputKnobs[0].knob, &outputKnobs[1].knob };
    for (int i = 0; i < 8; ++i)
        attachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.apvts, paramIds[i], *knobs[i]);

    footerLabel.setText("FL STUDIO · ABLETON · LOGIC · CUBASE · REAPER · BITWIG", juce::dontSendNotification);
    footerLabel.setFont(juce::FontOptions(9.5f).withStyle(juce::Font::bold));
    footerLabel.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.28f));
    addAndMakeVisible(footerLabel);
}

ReverbAudioProcessorEditor::~ReverbAudioProcessorEditor() = default;

void ReverbAudioProcessorEditor::setupKnob(KnobControl& kc, const char* caption)
{
    kc.caption.setText(caption, juce::dontSendNotification);
    kc.caption.setFont(juce::FontOptions(9.5f).withStyle(juce::Font::bold));
    kc.caption.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.40f));
    addAndMakeVisible(kc.caption);
    addAndMakeVisible(kc.knob);
}

void ReverbAudioProcessorEditor::layoutCardKnobs(const juce::Rectangle<int>& card,
                                                 KnobControl* knobs, int count)
{
    constexpr int knobH = 104;
    constexpr int verticalPad = 10;

    int y = card.getY() + 44;
    for (int i = 0; i < count; ++i)
    {
        auto block = juce::Rectangle<int>(card.getX(), y, card.getWidth(), knobH);
        knobs[i].caption.setBounds(block.removeFromTop(16));
        knobs[i].knob.setBounds(block.reduced(10, 0));
        y += knobH + verticalPad;
    }
}

void ReverbAudioProcessorEditor::applyPreset(int index)
{
    const char* paramIds[] = { "roomSize", "decay", "damping", "width",
                               "predelay", "mix", "loCut", "hiCut" };
    for (int p = 0; p < 8; ++p)
    {
        auto* param = audioProcessor.apvts.getParameter(paramIds[p]);
        if (param != nullptr)
            param->setValueNotifyingListeners(param->convertTo0to1(presets[index][p]));
    }
    updatePresetSelection(index);
}

void ReverbAudioProcessorEditor::updatePresetSelection(int index)
{
    for (int i = 0; i < presetButtons.size(); ++i)
        presetButtons[i]->setSelected(i == index);
}

void ReverbAudioProcessorEditor::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();

    g.setGradientFill(juce::ColourGradient(bgTop, 0.0f, 0.0f,
                                           bgBottom, 0.0f, (float)getHeight(), false));
    g.fillAll();

    g.setColour(cyan.withAlpha(0.04f));
    g.fillEllipse(bounds.getCentreX() - bounds.getWidth() * 0.4f, -bounds.getHeight() * 0.5f,
                  bounds.getWidth() * 0.8f, bounds.getHeight() * 0.9f);

    const char* cardNames[] = { "INPUT · TONE", "SPACE · ROOM", "OUTPUT · MIX" };
    for (int i = 0; i < 3; ++i)
    {
        auto rect = cardRects[i].toFloat();

        g.setColour(juce::Colour(0xFF141826).withAlpha(0.90f));
        g.fillRoundedRectangle(rect, 14.0f);
        g.setColour(juce::Colour(0x14FFFFFF));
        g.drawRoundedRectangle(rect, 14.0f, 1.0f);

        g.setFont(juce::FontOptions(10.0f).withStyle(juce::Font::bold));
        g.setColour(juce::Colours::white.withAlpha(0.40f));
        g.drawText(cardNames[i], rect.removeFromTop(40).reduced(16, 0),
                   juce::Justification::centredLeft);

        g.setColour(violet.withAlpha(0.35f));
        g.fillRoundedRectangle(rect.removeFromTop(2).removeFromLeft(36)
                                   .withSizeKeepingCentre(36, 2),
                               1.0f);
    }
}

void ReverbAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(28);

    auto header = area.removeFromTop(58);

    titleLabel.setBounds(header.removeFromLeft(220).removeFromTop(34));
    subtitleLabel.setBounds(header.removeFromLeft(200).removeFromTop(20).translated(0, 16));

    auto bypassArea = header.removeFromRight(130);
    bypassButton.setBounds(bypassArea.withSizeKeepingCentre(130, 28).translated(0, 8));

    versionLabel.setBounds(header.removeFromRight(180).removeFromTop(18).translated(0, 6));

    auto presetRow = area.removeFromTop(42);
    const float gap = 8.0f;
    const float btnWidth = (presetRow.getWidth() - gap * 5.0f) / 6.0f;
    for (int i = 0; i < 6; ++i)
    {
        presetButtons[i]->setBounds((int)(presetRow.getX() + i * (btnWidth + gap)),
                                    presetRow.getY(),
                                    (int)btnWidth,
                                    (int)(presetRow.getHeight() * 0.72f));
    }

    const int cardGap = 16;
    const int cardW = (area.getWidth() - 2 * cardGap) / 3;

    auto inputCard = area.removeFromLeft(cardW);
    area.removeFromLeft(cardGap);
    auto spaceCard = area.removeFromLeft(cardW);
    area.removeFromLeft(cardGap);
    auto outputCard = area.removeFromLeft(cardW);

    cardRects[0] = inputCard;
    cardRects[1] = spaceCard;
    cardRects[2] = outputCard;

    layoutCardKnobs(inputCard, inputKnobs, 3);
    layoutCardKnobs(spaceCard, spaceKnobs, 3);
    layoutCardKnobs(outputCard, outputKnobs, 2);

    footerLabel.setBounds(getLocalBounds().removeFromBottom(18).reduced(28, 0));
}
