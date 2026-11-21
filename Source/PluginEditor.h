#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class NeonKnob : public juce::Slider
{
public:
    NeonKnob()
    {
        setSliderStyle(juce::Slider::RotaryVerticalDrag);
        setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        setVelocityBasedMode(true);
        setVelocityModeParameters(0.8, 2, 0.08, false);
        setWantsKeyboardFocus(true);
        setMouseCursor(juce::MouseCursor::PointingHand);
    }

    void paint(juce::Graphics& g) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NeonKnob)
};

class PresetButton : public juce::Button
{
public:
    explicit PresetButton(const juce::String& name) : juce::Button(name)
    {
        setMouseCursor(juce::MouseCursor::PointingHand);
    }

    void setSelected(bool shouldBeSelected)
    {
        selected = shouldBeSelected;
        repaint();
    }

    void paintButton(juce::Graphics& g, bool highlighted, bool down) override;

private:
    bool selected = false;

    static const juce::Colour accent, accentBright, bgNormal, bgHover, bgSelected;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetetButton)
};

class PowerButton : public juce::ToggleButton
{
public:
    PowerButton()
    {
        setMouseCursor(juce::MouseCursor::PointingHand);
        setTooltip("Bypass the reverb");
    }

    void paintButton(juce::Graphics& g, bool highlighted, bool down) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PowerButton)
};

class ReverbAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit ReverbAudioProcessorEditor(ReverbAudioProcessor&);
    ~ReverbAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void setupKnob(struct KnobControl&, const char* caption);
    void layoutCardKnobs(const juce::Rectangle<int>&, struct KnobControl*, int count);
    void applyPreset(int index);
    void updatePresetSelection(int index);

    ReverbAudioProcessor& audioProcessor;

    struct KnobControl
    {
        NeonKnob knob;
        juce::Label caption;
    };

    KnobControl inputKnobs[3];
    KnobControl spaceKnobs[3];
    KnobControl outputKnobs[2];

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachments[8];

    juce::OwnedArray<PresetButton> presetButtons;
    juce::Label titleLabel, subtitleLabel, footerLabel, versionLabel;
    PowerButton bypassButton;

    juce::Rectangle<int> cardRects[3];

    static const juce::Colour bgTop, bgBottom, cyan, violet, magenta;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReverbAudioProcessorEditor)
};
