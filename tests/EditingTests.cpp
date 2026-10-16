#include "../src/PluginProcessor.h"
#include "../src/Music/Editing.h"
#include <limits>
#include <stdexcept>
#include <iostream>
namespace {void check(bool b,const char* s){if(!b)throw std::runtime_error(s);}}
void runEditingTests(){
 WeeProcessor p;auto original=p.snapshot();juce::MemoryBlock before;p.getContentState(before);
 auto edited=original;edited.notes.push_back(wee::editNote({},31.9,140,8,.5,8));check(edited.notes.back()==wee::Note{31.5,.5,127,90},"editing clamps grid/pitch/end");
 check(p.editLayer(0,edited),"valid edit commits");p.remember(before);check(p.canUndo(),"edit creates undo");auto committed=p.snapshot().notes;
 check(p.travelHistory(false)&&p.snapshot().notes==original.notes,"undo restores exact original");check(p.travelHistory(true)&&p.snapshot().notes==committed,"redo restores edit");
 juce::MemoryBlock saved;p.getStateInformation(saved);WeeProcessor reopened;reopened.setStateInformation(saved.getData(),int(saved.getSize()));check(reopened.canUndo()&&reopened.travelHistory(false)&&reopened.snapshot().notes==original.notes,"history survives project reload");
 {auto editor=std::unique_ptr<juce::AudioProcessorEditor>(p.createEditor());}check(p.canUndo(),"history survives editor close");
 auto bad=edited;bad.notes[0].beat=std::numeric_limits<double>::quiet_NaN();check(!p.editLayer(0,bad)&&p.snapshot().notes==committed,"invalid edits preserve content");
 auto* lock=p.params.getParameter(WeeProcessor::layerId(0,"lock"));lock->setValueNotifyingHost(1);check(!p.editLayer(0,original),"locked notes reject edits");lock->setValueNotifyingHost(0);
 check(!p.editLayer(-1,edited)&&!p.editLayer(8,edited),"invalid layer rejects edit");
 WeeProcessor preset;preset.generate();juce::MemoryBlock presetBytes;preset.getContentState(presetBytes);check(p.loadPreset(presetBytes)&&p.snapshot().notes==preset.snapshot().notes,"preset restores exact generated content");check(p.travelHistory(false)&&p.snapshot().notes==committed,"preset load is undoable");check(p.canRedo(),"preset redo exists");
 const char rubbish[]="invalid preset";juce::MemoryBlock invalid(rubbish,sizeof(rubbish));check(!p.loadPreset(invalid)&&p.snapshot().notes==committed,"invalid preset preserves unsaved work");
 p.preview=false;for(int n=0;n<12;++n){juce::MemoryBlock state;p.getContentState(state);p.generate();p.remember(std::move(state));}int count=0;while(p.travelHistory(false))++count;check(count==8,"history bounded to eight transactions");
 // Mood is deterministic, changes density/dynamics, and remains in scale.
 wee::Settings settings;settings.seed=987;settings.mood=1;auto reflective=wee::generate(settings);settings.mood=2;auto energetic=wee::generate(settings);check(reflective.notes!=energetic.notes,"mood changes generated notes");check(energetic.notes==wee::generate(settings).notes,"mood generation deterministic");
 auto* mood=p.params.getParameter("mood");mood->setValueNotifyingHost(1);p.generate();check(p.snapshot().settings.mood==2,"mood parameter drives generation");p.getStateInformation(saved);reopened.setStateInformation(saved.getData(),int(saved.getSize()));check(reopened.snapshot().settings.mood==2&&reopened.snapshot().notes==p.snapshot().notes,"mood and notes recalled");
 std::cout<<"PASS: transactional note edits, locks, persistent bounded history, preset undo/rejection and phrase moods\n";
}
