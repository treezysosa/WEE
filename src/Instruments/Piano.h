#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
struct PianoSound : juce::SynthesiserSound {
 bool appliesToNote(int) override{return true;}
 bool appliesToChannel(int) override{return true;}
};
// Eight original synthetic timbres. No sample libraries or recordings are used.
class InstrumentVoice : public juce::SynthesiserVoice {
public:
 explicit InstrumentVoice(std::atomic<float>* selection):instrument(selection){}
 bool canPlaySound(juce::SynthesiserSound* s) override{return dynamic_cast<PianoSound*>(s)!=nullptr;}
 void startNote(int note,float velocity,juce::SynthesiserSound*,int) override {
  kind=juce::jlimit(0,7,int(instrument->load()));phase=detunePhase=0;envelope=1;attack=0;level=velocity*0.11;released=false;
  double sr=getSampleRate(),hz=juce::MidiMessage::getMidiNoteInHertz(note);step=juce::MathConstants<double>::twoPi*hz/sr;
  double attackTime=kind==4?0.13:kind==5?0.4:kind==6?0.09:0.003;
  attackStep=1.0/(sr*attackTime);decay=std::exp(-(kind>=4&&kind<=6?0.035:kind==2?2.7:kind==1?1.0:1.6)/sr);
  release=std::exp(-(kind==4?5.0:kind==5?3.5:kind==6?8.0:28.0)/sr);
  transient=1;transientDecay=std::exp(-5.0/sr);weights.fill(0);
  for(int h=1;h<=12;++h){if(h*hz>sr*0.45)break;double w=0;
   switch(kind){
    case 0:w=h==1?1:h==2?0.32:h==3?0.12:0;break;
    case 1:w=h==1?1:h==2?0.1:0;break;
    case 2:w=std::pow(0.64,h-1)/std::sqrt(double(h));break;
    case 3:w=h==1?1:h==2?0.28:h==3?0.16:h==5?0.06:0;break;
    case 4:w=std::pow(0.78,h-1)/h;break;
    case 5:w=std::pow(0.45,h-1)/h;break;
    case 6:{double f=h*hz;w=(0.25+std::exp(-std::pow((f-700)/250,2))+0.7*std::exp(-std::pow((f-1200)/350,2)))/h;break;}
    case 7:w=h==1?1:h==2?0.45:h==3?0.18:h==4?0.09:0;break;
   }
   weights[size_t(h-1)]=w;
  }
 }
 void stopNote(float,bool tail) override{if(tail)released=true;else{envelope=0;clearCurrentNote();}}
 void pitchWheelMoved(int) override{} void controllerMoved(int,int) override{}
 void renderNextBlock(juce::AudioBuffer<float>& b,int start,int count) override {
  for(int i=0;i<count;++i){if(envelope<0.00005){clearCurrentNote();break;}
   double sample=0;
   if(kind==1)sample=std::sin(phase+1.8*transient*std::sin(2*phase));
   else for(int h=1;h<=12;++h)if(weights[size_t(h-1)]!=0)sample+=weights[size_t(h-1)]*std::sin(h*phase)*(kind==0&&h>1?transient:1);
   if(kind==4||kind==5)sample=0.8*sample+0.2*std::sin(detunePhase);
   float value=float(sample*envelope*std::min(1.0,attack)*level);
   for(int c=0;c<b.getNumChannels();++c)b.addSample(c,start+i,value);
   phase=std::fmod(phase+step,juce::MathConstants<double>::twoPi);detunePhase=std::fmod(detunePhase+step*1.003,juce::MathConstants<double>::twoPi);
   attack+=attackStep;envelope*=released?release:decay;transient*=transientDecay;
  }
 }
private:
 std::atomic<float>* instrument;int kind=0;bool released=false;
 std::array<double,12> weights{};
 double phase=0,detunePhase=0,step=0,envelope=0,attack=0,attackStep=0,decay=1,release=1,transient=1,transientDecay=1,level=0;
};
