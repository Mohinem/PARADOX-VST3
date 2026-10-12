#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "Engine/Engine.h"
#include <atomic>
namespace paradox {
inline constexpr auto gainParameterId = "output_gain";
static_assert(std::atomic<float>::is_always_lock_free);
class Processor final : public juce::AudioProcessor {
public:
    Processor();
    void prepareToPlay(double, int) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    bool hasEditor() const override { return true; }
    juce::AudioProcessorEditor* createEditor() override;
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;
    float peak() const noexcept { return outputPeak.load(std::memory_order_relaxed); }
    juce::AudioProcessorValueTreeState parameters;
private:
    LawEngine engine;
    EventBuffer events;
    std::atomic<float>* gain;
    std::atomic<float> outputPeak {};
};
}
