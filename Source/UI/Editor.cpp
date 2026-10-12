#include "Editor.h"
namespace paradox {
Editor::Editor(Processor& p) : AudioProcessorEditor(p), processor(p),
    attachment(p.parameters, gainParameterId, gainSlider) {
    title.setText(p.getName(), juce::dontSendNotification);
    gainSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 24);
    addAndMakeVisible(title); addAndMakeVisible(gainSlider); addAndMakeVisible(meter);
    setSize(400, 150); startTimerHz(20);
}
void Editor::resized() {
    auto area = getLocalBounds().reduced(16);
    title.setBounds(area.removeFromTop(30));
    gainSlider.setBounds(area.removeFromTop(40));
    meter.setBounds(area.removeFromTop(30));
}
void Editor::timerCallback() {
    meter.setText("Output peak: " + juce::String(processor.peak(), 3), juce::dontSendNotification);
}
}
