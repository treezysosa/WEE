#pragma once
#include "../PluginProcessor.h"
#include "../Music/Editing.h"
class WeePianoRoll : public juce::Component {
public:
 explicit WeePianoRoll(WeeProcessor& proc):p(proc){setWantsKeyboardFocus(true);setMouseCursor(juce::MouseCursor::CrosshairCursor);}
 std::function<void(int,const wee::Composition&)> onCommit;
 int layer=0;double grid=0.5;
 void paint(juce::Graphics& g)override{
  auto c=dragging?draft:p.snapshotArrangement().layers[size_t(layer)];
  g.fillAll(juce::Colours::white);
  for(int note=low;note<=high;++note){int pc=note%12;g.setColour(pc==1||pc==3||pc==6||pc==8||pc==10?juce::Colour(0xffedf3f8):juce::Colours::white);g.fillRect(0.f,y(note),float(getWidth()),rowHeight());}
  for(int b=0;b<=c.settings.bars*4;++b){g.setColour(b%4==0?juce::Colour(0xffa5cbdc):juce::Colour(0xffe0eaf0));g.drawVerticalLine(int(x(b,c.settings.bars)),0,float(getHeight()));}
  for(size_t i=0;i<c.notes.size();++i){auto n=c.notes[i];g.setColour(int(i)==selected?juce::Colour(0xff15699d):juce::Colour(0xff18b4e3).withAlpha(0.4f+n.velocity/220.f));g.fillRoundedRectangle(x(n.beat,c.settings.bars),y(n.pitch),std::max(2.f,x(n.length,c.settings.bars)),std::max(2.f,rowHeight()-1),2);}
 }
 void mouseDown(const juce::MouseEvent& e)override{
  grabKeyboardFocus();draft=p.snapshotArrangement().layers[size_t(layer)];selected=-1;baseline=draft.notes;
  for(int i=int(draft.notes.size())-1;i>=0;--i){auto n=draft.notes[size_t(i)];if(juce::Rectangle<float>(x(n.beat,draft.settings.bars),y(n.pitch),std::max(3.f,x(n.length,draft.settings.bars)),rowHeight()).contains(e.position)){selected=i;break;}}
  if(locked())return;
  if(e.mods.isRightButtonDown()){if(selected>=0){draft.notes.erase(draft.notes.begin()+selected);commit();}return;}
  dragging=true;start=e.position;
  if(selected<0){draft.notes.push_back(wee::editNote({},beat(e.x),pitchAt(e.y),grid,grid,draft.settings.bars));selected=int(draft.notes.size())-1;}
  original=draft.notes[size_t(selected)];resize=e.mods.isShiftDown();velocity=e.mods.isAltDown();repaint();
 }
 void mouseDrag(const juce::MouseEvent&e)override{
  if(!dragging||selected<0)return;auto n=original;
  if(velocity)n.velocity=std::clamp(original.velocity+int(start.y-e.position.y),1,127);
  else if(resize)n=wee::editNote(n,n.beat,n.pitch,n.length+beat(e.x)-beat(int(start.x)),grid,draft.settings.bars);
  else n=wee::editNote(n,n.beat+beat(e.x)-beat(int(start.x)),n.pitch+int(std::round((start.y-e.position.y)/rowHeight())),n.length,grid,draft.settings.bars);
  draft.notes[size_t(selected)]=n;repaint();
 }
 void mouseUp(const juce::MouseEvent&)override{if(dragging){dragging=false;commit();}}
 bool keyPressed(const juce::KeyPress& k)override{
  if(k==juce::KeyPress::escapeKey){dragging=false;repaint();return true;}
  if((k==juce::KeyPress::deleteKey||k==juce::KeyPress::backspaceKey)&&!locked()){
   auto c=p.snapshotArrangement().layers[size_t(layer)];if(selected>=0&&selected<int(c.notes.size())){c.notes.erase(c.notes.begin()+selected);if(onCommit)onCommit(layer,c);selected=-1;repaint();}return true;
  }return false;
 }
 void fit(){auto c=p.snapshotArrangement().layers[size_t(layer)];low=48;high=84;for(auto n:c.notes){low=std::min(low,n.pitch-2);high=std::max(high,n.pitch+2);}low=std::max(0,low);high=std::min(127,high);selected=-1;dragging=false;repaint();}
private:
 WeeProcessor&p;wee::Composition draft;wee::Note original;std::vector<wee::Note> baseline;int selected=-1,low=48,high=84;bool dragging=false,resize=false,velocity=false;juce::Point<float> start;
 bool locked()const{return p.params.getRawParameterValue(WeeProcessor::layerId(layer,"lock"))->load()>0.5f;}
 float rowHeight()const{return float(getHeight())/(high-low+1);}
 float y(int note)const{return (high-note)*rowHeight();}
 float x(double beat,int bars)const{return float(beat/(bars*4)*getWidth());}
 double beat(int px)const{return double(px)/std::max(1,getWidth())*draft.settings.bars*4;}
 int pitchAt(int py)const{return std::clamp(high-int(py/rowHeight()),0,127);}
 void commit(){dragging=false;if(onCommit&&p.snapshotArrangement().layers[size_t(layer)].notes==baseline&&wee::validNotes(draft))onCommit(layer,draft);selected=-1;repaint();}
};
