#pragma once
#include <JuceHeader.h>
#include "ReverbProcessor.h"

class ReverbAudioProcessor : public juce::AudioProcessor,
                             public juce::AudioProcessorValueTreeState::Listener
{
public:
    ReverbAudioProcessor();
    ~ReverbAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Aurora Reverb"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 4.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;
    void parameterChanged(const juce::String&, float) override {}

    ReverbProcessor* getReverbProcessor() const { return reverbEffect.get(); }

    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    std::unique_ptr<ReverbProcessor> reverbEffect;
    juce::AudioBuffer<float> scratchBuffer;

    std::atomic<float>* roomSizeParam = nullptr;
    std::atomic<float>* decayParam    = nullptr;
    std::atomic<float>* dampingParam  = nullptr;
    std::atomic<float>* widthParam    = nullptr;
    std::atomic<float>* predelayParam = nullptr;
    std::atomic<float>* mixParam      = nullptr;
    std::atomic<float>* loCutParam    = nullptr;
    std::atomic<float>* hiCutParam    = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReverbAudioProcessor)
};
