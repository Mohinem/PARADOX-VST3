#pragma once
#include "Plugin/Processor.h"
namespace paradox {
class Editor final : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit Editor(Processor&);
    void resized() override;
private:
    void timerCallback() override;
    Processor& processor;
    juce::Slider gainSlider;
    juce::Label title, meter;
    juce::AudioProcessorValueTreeState::SliderAttachment attachment;
};
}
