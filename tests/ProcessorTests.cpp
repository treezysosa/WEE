#include "../src/PluginProcessor.h"
#include <iostream>
#include <stdexcept>
static void require(bool b,const char* msg){if(!b)throw std::runtime_error(msg);}
static double energy(juce::AudioBuffer<float>& b){double e=0;for(int c=0;c<b.getNumChannels();++c)for(int i=0;i<b.getNumSamples();++i){auto s=b.getSample(c,i);require(std::isfinite(s),"non-finite audio");e+=s*s;}return e;}
int main(){juce::ScopedJuceInitialiser_GUI init;try{
 WeeProcessor p;p.prepareToPlay(48000,256);juce::AudioBuffer<float> buffer(2,256);juce::MidiBuffer midi;
 midi.addEvent(juce::MidiMessage::noteOn(1,69,(juce::uint8)110),0);p.processBlock(buffer,midi);require(energy(buffer)>0.001,"MIDI input must produce audible piano");
 auto original=p.snapshot();juce::MemoryBlock state;p.getStateInformation(state);p.generate();require(p.snapshot().notes!=original.notes,"regenerate must change content");
 p.setStateInformation(state.getData(),int(state.getSize()));require(p.snapshot().notes==original.notes,"state must restore exact notes");require(p.snapshot().settings.seed==original.settings.seed,"state must restore seed");
 WeeProcessor restored;restored.setStateInformation(state.getData(),int(state.getSize()));require(restored.snapshot().notes==original.notes,"fresh instance state recall");
 restored.prepareToPlay(44100,256);restored.preview=true;double sum=0;for(int i=0;i<800;++i){midi.clear();restored.processBlock(buffer,midi);sum+=energy(buffer);}require(sum>1,"preview must render audible melody");
 restored.preview=false;for(int i=0;i<300;++i){midi.clear();restored.processBlock(buffer,midi);}require(energy(buffer)<0.000001,"stop must release generated voices");
 std::cout<<"PASS: MIDI input piano, generation, state round-trip, preview audio and stop\n";
 }catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}return 0;}
