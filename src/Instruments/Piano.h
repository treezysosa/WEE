#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
struct PianoSound : juce::SynthesiserSound { bool appliesToNote(int) override{return true;} bool appliesToChannel(int) override{return true;} };
struct PianoVoice : juce::SynthesiserVoice {
 double phase=0, step=0, env=0, age=0; float velocity=0; bool released=false;
 bool canPlaySound(juce::SynthesiserSound* s) override{return dynamic_cast<PianoSound*>(s)!=nullptr;}
 void startNote(int n,float v,juce::SynthesiserSound*,int) override {phase=0;age=0;env=1;velocity=v;released=false;step=juce::MathConstants<double>::twoPi*juce::MidiMessage::getMidiNoteInHertz(n)/getSampleRate();}
 void stopNote(float,bool tail) override {if(tail)released=true;else{env=0;clearCurrentNote();}}
 void pitchWheelMoved(int) override{} void controllerMoved(int,int) override{}
 void renderNextBlock(juce::AudioBuffer<float>& b,int start,int count) override {
  for(int i=0;i<count;++i){if(env<0.00005){clearCurrentNote();break;}
   double attack=std::min(1.0,age*getSampleRate()/80.0);
   float s=float((std::sin(phase)+0.35*std::sin(phase*2.001)*std::exp(-age*3)+0.12*std::sin(phase*3.004)*std::exp(-age*6))*env*attack*velocity*0.15);
   for(int ch=0;ch<b.getNumChannels();++ch)b.addSample(ch,start+i,s);
   phase=std::fmod(phase+step,juce::MathConstants<double>::twoPi);age+=1/getSampleRate();env*=std::exp(-(released?30.0:1.6)/getSampleRate());
  }
 }
};
