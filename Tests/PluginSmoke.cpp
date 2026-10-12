#include <juce_audio_processors/juce_audio_processors.h>
#include <cmath>
#include <iostream>
#include <memory>
#include "AllocationProbe.h"
#include <stdexcept>
extern juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter();
void require(bool ok, const char* reason) { if (!ok) throw std::runtime_error(reason); }
int main(int argc, char** argv) {
    try {
        std::unique_ptr<juce::AudioProcessor> processor(createPluginFilter());
        require(processor && processor->acceptsMidi(), "factory/MIDI capability");
        require(processor->getTotalNumOutputChannels() == 2, "default stereo output");
        const bool instrument = processor->getTotalNumInputChannels() == 0;
        require(argc == 3 && processor->getName() == argv[1], "expected product identity");
        require(instrument == (juce::String(argv[2]) == "TRUE"), "expected instrument/effect bus role");
        auto* gain = dynamic_cast<juce::AudioProcessorParameterWithID*>(processor->getParameters()[0]);
        require(gain && gain->paramID == "output_gain", "stable gain identity");
        gain->setValueNotifyingHost(0.5f);
        juce::MemoryBlock state;
        processor->getStateInformation(state);
        gain->setValueNotifyingHost(0.1f);
        processor->setStateInformation(state.getData(), static_cast<int>(state.getSize()));
        require(std::abs(gain->getValue()-0.5f)<1e-6f, "plugin state restore");
        processor->setRateAndBufferSizeDetails(48000, 64);
        processor->prepareToPlay(48000, 64);
        juce::AudioBuffer<float> buffer(2,64);
        juce::MidiBuffer midi;
        buffer.clear();
        if (instrument) midi.addEvent(juce::MidiMessage::noteOn(1,69,1.0f), 8);
        else for (int c=0;c<2;++c) for(int s=0;s<64;++s) buffer.setSample(c,s,0.8f);
        probeActive = true;
        processor->processBlock(buffer,midi);
        probeActive = false;
        require(allocationCount == 0, "plugin callback C++ allocation probe");
        require(midi.isEmpty(), "no MIDI output");
        if (instrument) {
            require(buffer.getSample(0,7)==0 && buffer.getSample(0,9)!=0, "sample-timed instrument audio");
            midi.addEvent(juce::MidiMessage::noteOff(1,69),0);
            processor->processBlock(buffer,midi);
            require(buffer.getMagnitude(0,64)==0, "instrument note off");
        } else require(std::abs(buffer.getSample(1,63)-0.4f)<1e-6f, "effect input processing");
        auto layout = processor->getBusesLayout();
        layout.outputBuses.set(0,juce::AudioChannelSet::mono());
        if (!instrument) layout.inputBuses.set(0,juce::AudioChannelSet::mono());
        require(processor->checkBusesLayoutSupported(layout), "mono bus support");
        layout.outputBuses.set(0,juce::AudioChannelSet::create5point1());
        require(!processor->checkBusesLayoutSupported(layout), "unsupported bus rejected");
        processor->releaseResources();
        std::cout << processor->getName() << ": factory, buses, MIDI/audio, parameters and state passed\n";
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
