#pragma once
#include "../PluginProcessor.h"
#include "../Music/Editing.h"
class WeePianoRoll : public juce::Component,public juce::SettableTooltipClient {
public:
 explicit WeePianoRoll(WeeProcessor& proc):p(proc){setWantsKeyboardFocus(true);setMouseCursor(juce::MouseCursor::CrosshairCursor);}
 std::function<void(int,const wee::Composition&)> onCommit;
 int layer=0;double grid=0.5;
 void paint(juce::Graphics& g)override{
  const auto& c=dragging?draft:display();
  g.fillAll(juce::Colours::white);
  for(int note=low;note<=high;++note){int pc=note%12;g.setColour(pc==1||pc==3||pc==6||pc==8||pc==10?juce::Colour(0xffedf3f8):juce::Colours::white);g.fillRect(0.f,y(note),float(getWidth()),rowHeight());}
  for(int b=0;b<=c.settings.bars*4;++b){g.setColour(b%4==0?juce::Colour(0xffa5cbdc):juce::Colour(0xffe0eaf0));g.drawVerticalLine(int(x(b,c.settings.bars)),0,float(getHeight()));}
  for(size_t i=0;i<c.notes.size();++i){auto n=c.notes[i];g.setColour(int(i)==selected?juce::Colour(0xff15699d):juce::Colour(0xff18b4e3).withAlpha(0.4f+n.velocity/220.f));g.fillRoundedRectangle(x(n.beat,c.settings.bars),y(n.pitch),std::max(2.f,x(n.length,c.settings.bars)),std::max(2.f,rowHeight()-1),2);}
 }
 void mouseDown(const juce::MouseEvent& e)override{
  grabKeyboardFocus();draft=p.snapshotLayer(layer);selected=-1;baseline=draft.notes;gestureVersion=p.layerVersion(layer);
  for(int i=int(draft.notes.size())-1;i>=0;--i){auto n=draft.notes[size_t(i)];if(juce::Rectangle<float>(x(n.beat,draft.settings.bars),y(n.pitch),std::max(3.f,x(n.length,draft.settings.bars)),rowHeight()).contains(e.position)){selected=i;break;}}
  if(locked())return;
  if(e.mods.isRightButtonDown()){if(selected>=0){draft.notes.erase(draft.notes.begin()+selected);selected=-1;commit(false);}return;}
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
  if(k==juce::KeyPress::escapeKey){dragging=false;selected=-1;repaint();return true;}
  if(k==juce::KeyPress::deleteKey||k==juce::KeyPress::backspaceKey){
   if(locked())return true;
   auto c=p.snapshotLayer(layer);if(c.notes!=baseline){selected=-1;return true;}if(selected>=0&&selected<int(c.notes.size())){c.notes.erase(c.notes.begin()+selected);if(onCommit)onCommit(layer,c);selected=-1;repaint();}return true;
  }return false;
 }
 bool refresh(){auto version=p.layerVersion(layer);if(cachedLayer==layer&&cachedVersion==version)return false;cached=p.snapshotLayer(layer);cachedVersion=version;cachedLayer=layer;if(!dragging)selected=-1;repaint();return true;}
 const wee::Composition& display(){refresh();return cached;}
 void mouseMove(const juce::MouseEvent& e)override{
  if(locked()){setTooltip("Layer locked: unlock it in CREATE before editing notes.");return;}
  const auto& c=display();for(auto n:c.notes)if(juce::Rectangle<float>(x(n.beat,c.settings.bars),y(n.pitch),std::max(3.f,x(n.length,c.settings.bars)),rowHeight()).contains(e.position)){setTooltip("MIDI "+juce::String(n.pitch)+" | velocity "+juce::String(n.velocity)+" | beat "+juce::String(n.beat,3)+" | length "+juce::String(n.length,3));return;}
  setTooltip("Click: add | drag: move | Shift-drag: length | Alt-drag: velocity | right-click: delete | Escape: cancel");
 }
 void fit(){auto c=p.snapshotLayer(layer);low=48;high=84;for(auto n:c.notes){low=std::min(low,n.pitch-2);high=std::max(high,n.pitch+2);}low=std::max(0,low);high=std::min(127,high);selected=-1;dragging=false;repaint();}
private:
 WeeProcessor&p;wee::Composition cached,draft;int cachedLayer=-1;uint32_t cachedVersion=0,gestureVersion=0;wee::Note original;std::vector<wee::Note> baseline;int selected=-1,low=48,high=84;bool dragging=false,resize=false,velocity=false;juce::Point<float> start;
 bool locked()const{return p.params.getRawParameterValue(WeeProcessor::layerId(layer,"lock"))->load()>0.5f;}
 float rowHeight()const{return float(getHeight())/(high-low+1);}
 float y(int note)const{return (high-note)*rowHeight();}
 float x(double beat,int bars)const{return float(beat/(bars*4)*getWidth());}
 double beat(int px)const{return double(px)/std::max(1,getWidth())*draft.settings.bars*4;}
 int pitchAt(int py)const{return std::clamp(high-int(py/rowHeight()),0,127);}
 void commit(bool keepSelection=true){dragging=false;wee::Note target;bool keep=keepSelection&&selected>=0&&selected<int(draft.notes.size());if(keep)target=draft.notes[size_t(selected)];if(onCommit&&p.layerVersion(layer)==gestureVersion&&wee::validNotes(draft))onCommit(layer,draft);refresh();selected=-1;if(keep){auto it=std::find(cached.notes.begin(),cached.notes.end(),target);if(it!=cached.notes.end())selected=int(it-cached.notes.begin());}baseline=cached.notes;repaint();}
};
