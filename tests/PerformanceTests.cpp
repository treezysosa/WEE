#include "../src/PluginEditor.h"
#include <chrono>
#include <iostream>
#include <stdexcept>
static void need(bool condition,const char* text){if(!condition)throw std::runtime_error(text);}
static void setValue(WeeProcessor& p,const juce::String& id,float value){auto* parameter=p.params.getParameter(id);parameter->setValueNotifyingHost(parameter->convertTo0to1(value));}
void runPerformanceTests(){
 // Selection survives mouse-up, Delete works, and a stale selection cannot delete replacement content.
 WeeProcessor editorProcessor;auto c=editorProcessor.snapshot();c.notes={{4,.5,60,90}};need(editorProcessor.editLayer(0,c),"editor fixture commits");
 WeePianoRoll roll(editorProcessor);roll.setSize(1600,740);roll.onCommit=[&](int layer,const auto& content){editorProcessor.editLayer(layer,content);};roll.fit();
 auto mouse=[&](juce::Point<float> position){return juce::MouseEvent(juce::Desktop::getInstance().getMainMouseSource(),position,juce::ModifierKeys(juce::ModifierKeys::leftButtonModifier),1.f,0.f,0.f,0.f,0.f,&roll,&roll,juce::Time::getCurrentTime(),position,juce::Time::getCurrentTime(),1,false);};
 auto event=mouse({205,490});roll.mouseDown(event);roll.mouseUp(event);need(roll.keyPressed(juce::KeyPress(juce::KeyPress::deleteKey)),"Delete key handled");need(editorProcessor.snapshot().notes.empty(),"click selection persists for Delete");
 auto cancelEvent=mouse({405,490});roll.mouseDown(cancelEvent);roll.keyPressed(juce::KeyPress(juce::KeyPress::escapeKey));roll.mouseUp(cancelEvent);need(editorProcessor.snapshot().notes.empty(),"Escape cancels drafted note");
 c.notes={{4,.5,60,90}};editorProcessor.editLayer(0,c);roll.refresh();auto staleEvent=mouse({205,490});roll.mouseDown(staleEvent);roll.mouseUp(staleEvent);c.notes={{8,.5,64,80}};editorProcessor.editLayer(0,c);roll.keyPressed(juce::KeyPress(juce::KeyPress::deleteKey));need(editorProcessor.snapshot().notes==c.notes,"stale selection preserves new music");
 auto otherVersion=editorProcessor.layerVersion(1);c.notes[0].velocity=81;editorProcessor.editLayer(0,c);need(editorProcessor.layerVersion(1)==otherVersion,"editing one layer leaves other voice revisions intact");need(roll.refresh()&&roll.display().notes==c.notes,"cached display follows content revision");
 auto report=editorProcessor.diagnosticReport();need(report.startsWith(editorProcessor.getName()+" ")&&report.contains("FL Studio")&&!report.contains("C:\\Users\\"),"local diagnostic report fields");
 // Report measured callback costs; shared-runner timing is not a real-time certification gate.
 for(double rate:{44100.,48000.,96000.})for(int block:{64,256,1024})for(bool fx:{false,true}){
  WeeProcessor p;for(int layer=0;layer<8;++layer)setValue(p,WeeProcessor::layerId(layer,"enabled"),1);setValue(p,"fx_enable",fx?1.f:0.f);setValue(p,"fx_reverb",.35f);setValue(p,"fx_chorus",.2f);p.setRateAndBufferSizeDetails(rate,block);p.prepareToPlay(rate,block);p.preview=true;juce::AudioBuffer<float> audio(2,block);juce::MidiBuffer midi;std::vector<double> costs;costs.reserve(160);double energy=0;
  for(int iteration=0;iteration<180;++iteration){auto start=std::chrono::steady_clock::now();p.processBlock(audio,midi);double us=std::chrono::duration<double,std::micro>(std::chrono::steady_clock::now()-start).count();if(iteration>=20)costs.push_back(us);for(int sample=0;sample<block;++sample){float value=audio.getSample(0,sample);need(std::isfinite(value),"stress audio finite");energy+=value*value;}}
  need(energy>0,"stress playback audible");need(p.meterPeak[0].exchange(0)>0,"output meter records actual audio");std::sort(costs.begin(),costs.end());double budget=block/rate*1.e6;
  std::cout<<"PROFILE rate="<<rate<<" block="<<block<<" layers=8 fx="<<fx<<" p50_us="<<costs[costs.size()/2]<<" p99_us="<<costs[costs.size()*99/100]<<" budget_us="<<budget<<'\n';
 }
 std::cout<<"PASS: real note-selection gestures, stale protection, revision cache, diagnostics, meters and 18 callback profiles\n";
}
