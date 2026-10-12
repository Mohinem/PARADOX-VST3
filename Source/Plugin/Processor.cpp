#include "Processor.h"
#include "UI/Editor.h"
#include "Persistence/State.h"
namespace paradox {
Processor::Processor()
    : AudioProcessor(BusesProperties()
#if !JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "PARADOXParameters", {
          std::make_unique<juce::AudioParameterFloat>(juce::ParameterID {gainParameterId, 1},
              "Output Gain", juce::NormalisableRange<float> {0.0f, 1.0f}, 0.2f)}),
      gain(parameters.getRawParameterValue(gainParameterId)) {}
void Processor::prepareToPlay(double rate, int) { engine.prepare(rate); }
bool Processor::isBusesLayoutSupported(const BusesLayout& layout) const {
    const auto out = layout.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo()) return false;
#if JucePlugin_IsSynth
    return layout.getMainInputChannelSet().isDisabled();
#else
    return layout.getMainInputChannelSet() == out;
#endif
}
void Processor::processBlock(juce::AudioBuffer<float>& audio, juce::MidiBuffer& midi) {
    juce::ScopedNoDenormals noDenormals;
    events.clear();
    // Inspect raw bytes: no owning MidiMessage construction (including SysEx).
    for (const auto metadata : midi) {
        if (metadata.samplePosition < 0 || metadata.samplePosition >= audio.getNumSamples() || metadata.numBytes < 1) continue;
        const auto* b = metadata.data;
        const auto status = b[0] & 0xf0;
        Event e {static_cast<std::uint32_t>(metadata.samplePosition), EventType::allNotesOff,
                 0, static_cast<std::uint8_t>(b[0] & 0x0f), 0};
        if ((status == 0x90 || status == 0x80) && metadata.numBytes >= 3) {
            e.type = status == 0x90 && b[2] != 0 ? EventType::noteOn : EventType::noteOff;
            e.note = b[1]; e.velocity = b[2] / 127.0f;
        } else if (!(status == 0xb0 && metadata.numBytes >= 3 && (b[1] == 120 || b[1] == 123))) continue;
        if (!events.push(e)) break;
    }
    // Bounded overload policy: reject the block's MIDI and silence all voices.
    if (events.overflowed()) { events.clear(); events.push({0, EventType::allNotesOff, 0, 255, 0}); }
    engine.setGain(gain->load(std::memory_order_relaxed));
    engine.process(JucePlugin_IsSynth ? Backend::instrument : Backend::effect,
                   audio.getArrayOfWritePointers(), audio.getNumChannels(), audio.getNumSamples(), events.view());
    float peak = 0;
    for (int c = 0; c < audio.getNumChannels(); ++c)
        for (int s = 0; s < audio.getNumSamples(); ++s) peak = std::max(peak, std::abs(audio.getSample(c, s)));
    outputPeak.store(peak, std::memory_order_relaxed);
    midi.clear();
}
juce::AudioProcessorEditor* Processor::createEditor() { return new Editor(*this); }
void Processor::getStateInformation(juce::MemoryBlock& dest) {
    const auto bytes = encodeState({gain->load(std::memory_order_relaxed)});
    dest.replaceAll(bytes.data(), bytes.size());
}
void Processor::setStateInformation(const void* data, int size) {
    if (!data || size < 0) return;
    const auto model = decodeState({static_cast<const std::uint8_t*>(data), static_cast<std::size_t>(size)});
    if (!model) return;
    if (auto* parameter = parameters.getParameter(gainParameterId))
        parameter->setValueNotifyingHost(parameter->convertTo0to1(model->gain));
}
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new paradox::Processor(); }
