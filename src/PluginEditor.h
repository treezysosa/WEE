#pragma once
#include "PluginProcessor.h"
class MidiDragButton : public juce::TextButton {
public:
 MidiDragButton():juce::TextButton("DRAG MIDI"){}
 std::function<void()> onFileDrag;
 void mouseDown(const juce::MouseEvent&e)override{started=false;juce::TextButton::mouseDown(e);}
 void mouseDrag(const juce::MouseEvent&e)override{if(!started&&e.getDistanceFromDragStart()>=6&&onFileDrag){started=true;onFileDrag();}}
private:bool started=false;
};
class LayerRow : public juce::Component {
public:
 using CA=juce::AudioProcessorValueTreeState::ComboBoxAttachment;
 using SA=juce::AudioProcessorValueTreeState::SliderAttachment;
 using BA=juce::AudioProcessorValueTreeState::ButtonAttachment;
 LayerRow(WeeProcessor&,int);
 juce::TextButton name,regenerate{"REGEN"},midi{"MIDI"};
 juce::ToggleButton enabled{"ON"},mute{"M"},solo{"S"},lock{"LOCK"};
 juce::ComboBox instrument;juce::Slider volume,pan;
 void resized() override;
private:
 std::array<std::unique_ptr<BA>,4> buttons;
 std::unique_ptr<CA> instrumentAttachment;
 std::unique_ptr<SA> volumeAttachment,panAttachment;
};
class WeeEditor : public juce::AudioProcessorEditor,private juce::Timer {
public:
 explicit WeeEditor(WeeProcessor&);~WeeEditor()override{stopTimer();setLookAndFeel(nullptr);}
 void paint(juce::Graphics&)override;void resized()override;
private:
 WeeProcessor&p;juce::LookAndFeel_V4 look;
 juce::ComboBox style,key,scale,bars;juce::Slider bpm,volume;juce::ToggleButton sync{"Host sync"};
 juce::TextButton generate{"GENERATE"},play{"PLAY"},save{"SAVE ALL MIDI"},undo{"UNDO"},redo{"REDO"};MidiDragButton drag;
 juce::Component mixerContent;juce::Viewport mixerViewport;
 std::array<std::unique_ptr<LayerRow>,8> rows;
 using CA=juce::AudioProcessorValueTreeState::ComboBoxAttachment;using SA=juce::AudioProcessorValueTreeState::SliderAttachment;
 std::unique_ptr<CA>aStyle,aKey,aScale,aBars;std::unique_ptr<SA>aBpm,aVolume;std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>aSync;
 std::unique_ptr<juce::FileChooser> chooser;
 std::vector<juce::MemoryBlock>past,future;
 int selectedLayer=0;juce::Rectangle<float> roll;
 void timerCallback()override;
 void exportMidi(bool,int layer=-1);
 void compose(int layer=-1);
 void history(bool redo);
};
