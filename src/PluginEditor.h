#pragma once
#include "PluginProcessor.h"
#include "Export/Renderer.h"
#include "UI/PianoRoll.h"
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
 juce::TextButton name,regenerate{"REGEN"},midi{"MIDI"},wav{"WAV"};
 juce::ToggleButton enabled{"ON"},mute{"M"},solo{"S"},lock{"LOCK"};
 juce::ComboBox instrument;juce::Slider volume,pan;
 void resized() override;
private:
 std::array<std::unique_ptr<BA>,4> buttons;
 std::unique_ptr<CA> instrumentAttachment;
 std::unique_ptr<SA> volumeAttachment,panAttachment;
};
class EffectControl : public juce::Component {
public:
 EffectControl(WeeProcessor&,const wee::EffectParameter&);
 void resized() override;
private:
 juce::Label label;juce::Slider slider;juce::ToggleButton toggle;juce::ComboBox choice;
 std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sliderAttachment;
 std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> buttonAttachment;
 std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> choiceAttachment;
 int kind;
};
class WeeEditor : public juce::AudioProcessorEditor,private juce::Timer {
public:
 explicit WeeEditor(WeeProcessor&);~WeeEditor()override{stopTimer();renderJob.cancel();setLookAndFeel(nullptr);}
 void paint(juce::Graphics&)override;void resized()override;
private:
 WeeProcessor&p;juce::LookAndFeel_V4 look;
 WeeRenderJob renderJob;
 juce::TextButton createPage{"CREATE / MIXER"},effectsPage{"FX / FLIP / WAV"};
 bool showEffects=false;
 juce::Viewport effectViewport;juce::Component effectContent;
 std::vector<std::unique_ptr<EffectControl>> effectControls;
 std::array<juce::Label,3> sectionLabels;
 juce::TextButton renderAB{"RENDER A/B"},original{"ORIGINAL"},flipped{"FLIPPED"},live{"LIVE"},saveWav{"SAVE WAV"},stems{"EXPORT STEMS"},openFolder{"OPEN EXPORT FOLDER"},cancelRender{"CANCEL"};
 MidiDragButton dragWav;juce::Label renderStatus;
 double renderProgress=0;juce::ProgressBar progressBar{renderProgress};
 void setPage(bool);void exportAudio(int layer=-1,bool stems=false);void renderAudition();
 void layoutEffects(int width);

 juce::ComboBox style,key,scale,bars;juce::Slider bpm,volume;juce::ToggleButton sync{"Host sync"};
 juce::TextButton generate{"GENERATE"},play{"PLAY"},save{"SAVE ALL MIDI"},undo{"UNDO"},redo{"REDO"};MidiDragButton drag;
 juce::Component mixerContent;juce::Viewport mixerViewport;
 std::array<std::unique_ptr<LayerRow>,8> rows;
 using CA=juce::AudioProcessorValueTreeState::ComboBoxAttachment;using SA=juce::AudioProcessorValueTreeState::SliderAttachment;
 std::unique_ptr<CA>aStyle,aKey,aScale,aBars;std::unique_ptr<SA>aBpm,aVolume;std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>aSync;
 std::unique_ptr<juce::FileChooser> chooser;
 WeePianoRoll pianoRoll;
 juce::TextButton editPage{"EDIT / LIBRARY"},presetSave{"SAVE PRESET"},presetLoad{"LOAD"},presetFile{"OPEN .wee"},presetFolder{"PRESET FOLDER"},fitNotes{"FIT NOTES"};
 juce::ComboBox library,grid,mood;
 std::unique_ptr<CA> aMood;
 juce::Label editHelp;
 std::vector<juce::File> presetFiles;
 bool editing=false;
 void refreshLibrary();void savePreset();void loadPresetFile();void loadSelectedPreset();
 static juce::File presetDirectory();

 int selectedLayer=0;juce::Rectangle<float> roll;
 void timerCallback()override;
 void exportMidi(bool,int layer=-1);
 void compose(int layer=-1);
 void history(bool redo);
};
