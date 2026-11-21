#pragma once
#include <JuceHeader.h>

class ReverbProcessor
{
public:
    ReverbProcessor() = default;
    ~ReverbProcessor() = default;

    void prepare(double sampleRate, int maximumBlockSize);
    void reset();
    void setBypassed(bool shouldBypass) noexcept { bypassed = shouldBypass; }

    void process(const float* inL, const float* inR,
                 float* outL, float* outR,
                 int numSamples);

    void setParameters(float roomSize, float decaySeconds, float damping,
                       float width, float predelayMs, float mix,
                       float loCutHz, float hiCutHz);

private:
    struct CombFilter
    {
        std::vector<float> delayLine;
        int writeIndex = 0;
        int length = 0;
        float lastLowpass = 0.0f;

        void reset()
        {
            std::fill(delayLine.begin(), delayLine.end(), 0.0f);
            writeIndex = 0;
            lastLowpass = 0.0f;
        }

        void setLength(int newLength)
        {
            if (newLength == length)
                return;
            delayLine.resize(newLength, 0.0f);
            length = newLength;
            writeIndex %= length;
        }

        float process(float input, float feedback, float damping)
        {
            const float output = delayLine[writeIndex];
            lastLowpass = output * (1.0f - damping) + lastLowpass * damping;
            delayLine[writeIndex] = input + lastLowpass * feedback;
            if (++writeIndex >= length) writeIndex = 0;
            return output;
        }
    };

    struct AllpassFilter
    {
        std::vector<float> delayLine;
        int writeIndex = 0;
        int length = 0;

        void reset()
        {
            std::fill(delayLine.begin(), delayLine.end(), 0.0f);
            writeIndex = 0;
        }

        void setLength(int newLength)
        {
            if (newLength == length)
                return;
            delayLine.resize(newLength, 0.0f);
            length = newLength;
            writeIndex %= length;
        }

        float process(float input, float feedback)
        {
            const float bufout = delayLine[writeIndex];
            const float output = -input + bufout;
            delayLine[writeIndex] = input + bufout * feedback;
            if (++writeIndex >= length) writeIndex = 0;
            return output;
        }
    };

    void applyDelaySizes(float roomSize);
    int samplesForDelay(float milliseconds, float scale) const noexcept;
    float finiteFeedback(float delaySeconds, float decaySeconds) const noexcept;
    void updateInputFilters();

    double sampleRate = 44100.0;
    bool bypassed = false;

    CombFilter combsL[4], combsR[4];
    AllpassFilter allpassesL[2], allpassesR[2];

    juce::AudioBuffer<float> predelayBuffer;
    int predelayPosition = 0;
    int predelaySamples = 0;

    juce::LinearSmoothedValue roomSizeSmooth, decaySmooth, dampingSmooth,
                              widthSmooth, predelaySmooth, mixSmooth,
                              loCutSmooth, hiCutSmooth;

    juce::dsp::IIR::Filter<float> hpFilterL, hpFilterR, lpFilterL, lpFilterR;

    float lastInL = 0.0f, lastOutL = 0.0f;
    float lastInR = 0.0f, lastOutR = 0.0f;
};
