#include "ReverbProcessor.h"

namespace
{
    constexpr float combLMs[] = { 25.3f, 26.9f, 29.0f, 30.7f };
    constexpr float combRMs[] = { 25.6f, 27.3f, 29.3f, 31.1f };
    constexpr float apLMs[]   = { 12.6f, 10.0f };
    constexpr float apRMs[]   = { 12.7f, 10.1f };

    constexpr float allpassFeedback = 0.5f;

    constexpr float defaultRoomSize = 0.70f;
    constexpr float defaultDecay    = 2.80f;
    constexpr float defaultDamping  = 0.50f;
    constexpr float defaultWidth    = 0.60f;
    constexpr float defaultPredelay = 25.0f;
    constexpr float defaultMix      = 0.30f;
    constexpr float defaultLoCut    = 90.0f;
    constexpr float defaultHiCut    = 10000.0f;
}

int ReverbProcessor::samplesForDelay(float ms, float scale) const noexcept
{
    return jmax(1, (int)(ms * 0.001f * scale * (float)sampleRate));
}

float ReverbProcessor::finiteFeedback(float delaySeconds, float decaySeconds) const noexcept
{
    const float safeDecay = jmax(0.1f, decaySeconds);
    const float fb = std::pow(10.0f, -3.0f * delaySeconds / safeDecay);
    return jlimit(0.0f, 0.985f, fb);
}

void ReverbProcessor::applyDelaySizes(float roomSize)
{
    const float scale = 0.25f + roomSize * 0.75f;

    for (int i = 0; i < 4; ++i)
    {
        combsL[i].setLength(samplesForDelay(combLMs[i], scale));
        combsR[i].setLength(samplesForDelay(combRMs[i], scale));
    }
    for (int i = 0; i < 2; ++i)
    {
        allpassesL[i].setLength(samplesForDelay(apLMs[i], scale));
        allpassesR[i].setLength(samplesForDelay(apRMs[i], scale));
    }
}

void ReverbProcessor::updateInputFilters()
{
    const float loCutHz = loCutSmooth.getCurrentValue();
    const float hiCutHz = hiCutSmooth.getCurrentValue();

    auto hpCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass((float)sampleRate, loCutHz);
    auto lpCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowPass((float)sampleRate, hiCutHz);

    hpFilterL.coefficients = hpCoeffs;
    hpFilterR.coefficients = hpCoeffs;
    lpFilterL.coefficients = lpCoeffs;
    lpFilterR.coefficients = lpCoeffs;
}

void ReverbProcessor::prepare(double newSampleRate, int maximumBlockSize)
{
    sampleRate = newSampleRate;

    applyDelaySizes(defaultRoomSize);

    const double rampTime = 0.05;
    roomSizeSmooth.reset(sampleRate, rampTime);
    decaySmooth.reset(sampleRate, rampTime);
    dampingSmooth.reset(sampleRate, rampTime);
    widthSmooth.reset(sampleRate, rampTime);
    predelaySmooth.reset(sampleRate, rampTime);
    mixSmooth.reset(sampleRate, rampTime);
    loCutSmooth.reset(sampleRate, rampTime);
    hiCutSmooth.reset(sampleRate, rampTime);

    roomSizeSmooth.setCurrentAndTargetValue(defaultRoomSize);
    decaySmooth.setCurrentAndTargetValue(defaultDecay);
    dampingSmooth.setCurrentAndTargetValue(defaultDamping);
    widthSmooth.setCurrentAndTargetValue(defaultWidth);
    predelaySmooth.setCurrentAndTargetValue(defaultPredelay);
    mixSmooth.setCurrentAndTargetValue(defaultMix);
    loCutSmooth.setCurrentAndTargetValue(defaultLoCut);
    hiCutSmooth.setCurrentAndTargetValue(defaultHiCut);

    hpFilterL.reset(); hpFilterR.reset();
    lpFilterL.reset(); lpFilterR.reset();
    updateInputFilters();

    const int maxPredelaySamples = (int)(0.25 * sampleRate) + 8;
    predelayBuffer.setSize(2, maxPredelaySamples);
    predelayBuffer.clear();
    predelayPosition = 0;
    predelaySamples = 0;

    lastInL = lastOutL = lastInR = lastOutR = 0.0f;
}

void ReverbProcessor::reset()
{
    for (auto& c : combsL) c.reset();
    for (auto& c : combsR) c.reset();
    for (auto& a : allpassesL) a.reset();
    for (auto& a : allpassesR) a.reset();

    predelayBuffer.clear();
    predelayPosition = 0;

    hpFilterL.reset(); hpFilterR.reset();
    lpFilterL.reset(); lpFilterR.reset();

    lastInL = lastOutL = lastInR = lastOutR = 0.0f;
}

void ReverbProcessor::setParameters(float roomSize, float decaySeconds, float damping,
                                    float width, float predelayMs, float mix,
                                    float loCutHz, float hiCutHz)
{
    roomSizeSmooth.setTargetValue(roomSize);
    decaySmooth.setTargetValue(decaySeconds);
    dampingSmooth.setTargetValue(damping);
    widthSmooth.setTargetValue(width);
    predelaySmooth.setTargetValue(predelayMs);
    mixSmooth.setTargetValue(mix);
    loCutSmooth.setTargetValue(loCutHz);
    hiCutSmooth.setTargetValue(hiCutHz);
}

void ReverbProcessor::process(const float* inL, const float* inR,
                              float* outL, float* outR, int numSamples)
{
    if (bypassed)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            outL[i] = inL[i];
            outR[i] = inR[i];
        }
        return;
    }

    updateInputFilters();
    loCutSmooth.skip(numSamples);
    hiCutSmooth.skip(numSamples);

    const int predelayBufSize = predelayBuffer.getNumSamples();

    for (int i = 0; i < numSamples; ++i)
    {
        const float dryL = inL[i];
        const float dryR = inR[i];

        const float sizeNow  = roomSizeSmooth.getNextValue();
        const float decayNow = decaySmooth.getNextValue();
        const float dampNow  = dampingSmooth.getNextValue();
        const float widthNow = widthSmooth.getNextValue();

        predelayBuffer.setSample(0, predelayPosition, dryL);
        predelayBuffer.setSample(1, predelayPosition, dryR);

        const float predelayMsNow = predelaySmooth.getNextValue();
        const int newPredelaySamples = (int)(predelayMsNow * 0.001f * (float)sampleRate);
        predelaySamples = jmin(newPredelaySamples, predelayBufSize - 1);

        int readIndex = predelayPosition + predelaySamples;
        if (readIndex >= predelayBufSize) readIndex -= predelayBufSize;

        float wetL = predelayBuffer.getSample(0, readIndex);
        float wetR = predelayBuffer.getSample(1, readIndex);

        if (++predelayPosition >= predelayBufSize) predelayPosition = 0;

        wetL = hpFilterL.processSample(wetL);
        wetR = hpFilterR.processSample(wetR);
        wetL = lpFilterL.processSample(wetL);
        wetR = lpFilterR.processSample(wetR);

        applyDelaySizes(sizeNow);

        float sumL = 0.0f, sumR = 0.0f;

        for (int c = 0; c < 4; ++c)
        {
            const float delaySeconds  = (float)combsL[c].length / (float)sampleRate;
            const float delaySecondsR = (float)combsR[c].length / (float)sampleRate;
            const float fb  = finiteFeedback(delaySeconds, decayNow);
            const float fbR = finiteFeedback(delaySecondsR, decayNow);

            sumL += combsL[c].process(wetL, fb, dampNow);
            sumR += combsR[c].process(wetR, fbR, dampNow);
        }

        for (int c = 0; c < 2; ++c)
        {
            sumL = allpassesL[c].process(sumL, allpassFeedback);
            sumR = allpassesR[c].process(sumR, allpassFeedback);
        }

        wetL = sumL * 0.25f;
        wetR = sumR * 0.25f;

        {
            const float dcL = wetL - lastInL + 0.995f * lastOutL;
            lastInL = wetL; lastOutL = dcL; wetL = dcL;

            const float dcR = wetR - lastInR + 0.995f * lastOutR;
            lastInR = wetR; lastOutR = dcR; wetR = dcR;
        }

        {
            const float mid  = (wetL + wetR) * 0.5f;
            const float side = (wetL - wetR) * 0.5f;
            wetL = mid + side * widthNow;
            wetR = mid - side * widthNow;
        }

        const float mixNow = mixSmooth.getNextValue();
        const float dryGain = std::cos(mixNow * juce::MathConstants<float>::halfPi);
        const float wetGain = std::sin(mixNow * juce::MathConstants<float>::halfPi);

        outL[i] = dryL * dryGain + wetL * wetGain;
        outR[i] = dryR * dryGain + wetR * wetGain;
    }
}
