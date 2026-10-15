#pragma once
#include "PluginProcessor.h"
class MidiDragButton : public juce::TextButton {
public:
 MidiDragButton() : juce::TextButton("DRAG MIDI") { setTooltip("Drag a MIDI file into FL Studio; use SAVE MIDI if the host rejects file dragging."); }
 std::function<void()> onFileDrag;
 void mouseDown(const juce::MouseEvent& event) override { started = false; juce::TextButton::mouseDown(event); }
 void mouseDrag(const juce::MouseEvent& event) override {
  if (!started && event.getDistanceFromDragStart() >= 6 && onFileDrag) { started = true; onFileDrag(); }
 }
private:
 bool started = false;
};
class WeeEditor : public juce::AudioProcessorEditor,private juce::Timer {
public: explicit WeeEditor(WeeProcessor&); ~WeeEditor() override { stopTimer(); setLookAndFeel(nullptr); }void paint(juce::Graphics&) override;void resized() override;
private:
 WeeProcessor& p;juce::LookAndFeel_V4 look;
 juce::ComboBox style,key,scale,bars;juce::Slider bpm,volume;juce::ToggleButton sync{"Host sync"};
 juce::TextButton generate{"GENERATE"},play{"PLAY / STOP"},save{"SAVE MIDI"},undo{"UNDO"},redo{"REDO"};
 MidiDragButton drag;
 using CA=juce::AudioProcessorValueTreeState::ComboBoxAttachment;using SA=juce::AudioProcessorValueTreeState::SliderAttachment;
 std::unique_ptr<CA> aStyle,aKey,aScale,aBars;std::unique_ptr<SA> aBpm,aVolume;std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> aSync;
 std::unique_ptr<juce::FileChooser> chooser;std::vector<wee::Composition> past,future;
 void timerCallback() override{repaint();}void exportMidi(bool);
};
