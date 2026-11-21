#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout ReverbAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "roomSize", "Room Size",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.70f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "decay", "Decay",
        juce::NormalisableRange<float>(0.1f, 10.0f, 0.01f, 0.5f), 2.80f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "damping", "Damping",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.50f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "width", "Width",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.60f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "predelay", "Pre-Delay",
        juce::NormalisableRange<float>(0.0f, 250.0f, 0.5f), 25.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "mix", "Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.30f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "loCut", "Low Cut",
        juce::NormalisableRange<float>(20.0f, 500.0f, 1.0f, 0.6f), 90.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "hiCut", "High Cut",
        juce::NormalisableRange<float>(1000.0f, 20000.0f, 1.0f, 0.5f), 10000.0f));

    return layout;
}

ReverbAudioProcessor::ReverbAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMS", createParameterLayout())
{
    reverbEffect = std::make_unique<ReverbProcessor>();

    roomSizeParam = apvts.getRawParameterValue("roomSize");
    decayParam    = apvts.getRawParameterValue("decay");
    dampingParam  = apvts.getRawParameterValue("damping");
    widthParam    = apvts.getRawParameterValue("width");
    predelayParam = apvts.getRawParameterValue("predelay");
    mixParam      = apvts.getRawParameterValue("mix");
    loCutParam    = apvts.getRawParameterValue("loCut");
    hiCutParam    = apvts.getRawParameterValue("hiCut");
}

ReverbAudioProcessor::~ReverbAudioProcessor() = default;

void ReverbAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
#if JUCE_MAJOR_VERSION >= 8
    const int maxBlock = getPlaybackConfig().getMaxSamplesPerBlock();
#else
    const int maxBlock = getBlockSize();
#endif

    reverbEffect->prepare(sampleRate, maxBlock);
    scratchBuffer.setSize(2, maxBlock);
}

void ReverbAudioProcessor::releaseResources()
{
    reverbEffect->reset();
}

bool ReverbAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainInputChannelSet() == juce::AudioChannelSet::disabled())
        return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::disabled();

    const int numIn  = layouts.getMainInputChannelSet().size();
    const int numOut = layouts.getMainOutputChannelSet().size();
    return numIn <= 2 && numOut <= 2;
}

void ReverbAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    const int numInputChannels  = getTotalNumInputChannels();
    const int numOutputChannels = getTotalNumOutputChannels();

    for (int ch = numInputChannels; ch < numOutputChannels; ++ch)
        buffer.clear(ch, 0, numSamples);

    const float* inL = buffer.getReadPointer(0);
    const float* inR = numInputChannels > 1 ? buffer.getReadPointer(1) : inL;

    float* outL = buffer.getWritePointer(0);
    float* outR = numOutputChannels > 1 ? buffer.getWritePointer(1) : nullptr;

    reverbEffect->setParameters(
        roomSizeParam->load(), decayParam->load(), dampingParam->load(),
        widthParam->load(), predelayParam->load(), mixParam->load(),
        loCutParam->load(), hiCutParam->load());

    if (outR != nullptr)
    {
        reverbEffect->process(inL, inR, outL, outR, numSamples);
    }
    else
    {
        reverbEffect->process(inL, inR, outL, scratchBuffer.getWritePointer(1), numSamples);
    }
}

juce::AudioProcessorEditor* ReverbAudioProcessor::createEditor()
{
    return new ReverbAudioProcessorEditor(*this);
}

void ReverbAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void ReverbAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}
