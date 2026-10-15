#include "../src/PluginProcessor.h"
#include "../src/Export/Midi.h"
#include <iostream>
#include <stdexcept>
static void require(bool b,const char* msg){if(!b)throw std::runtime_error(msg);}
static double energy(juce::AudioBuffer<float>& b){double e=0;for(int c=0;c<b.getNumChannels();++c)for(int i=0;i<b.getNumSamples();++i){auto s=b.getSample(c,i);require(std::isfinite(s),"non-finite audio");e+=s*s;}return e;}
struct TestPlayHead : juce::AudioPlayHead {
 PositionInfo position;
 juce::Optional<PositionInfo> getPosition() const override { return position; }
};
int main(){juce::ScopedJuceInitialiser_GUI init;try{
 WeeProcessor p;p.setRateAndBufferSizeDetails(48000,256);p.prepareToPlay(48000,256);juce::AudioBuffer<float> buffer(2,256);juce::MidiBuffer midi;
 midi.addEvent(juce::MidiMessage::noteOn(1,69,(juce::uint8)110),0);p.processBlock(buffer,midi);require(energy(buffer)>0.001,"MIDI input must produce audible piano");
 p.params.getParameter("gain")->setValueNotifyingHost(0.33f);
 auto original=p.snapshot();
 auto bytes=wee::midi(original);juce::MemoryInputStream stream(bytes.data(),bytes.size(),false);juce::MidiFile parsed;
 require(parsed.readFrom(stream),"JUCE must independently parse exported SMF");
 require(parsed.getTimeFormat()==960 && parsed.getNumTracks()==1,"SMF PPQ/track metadata");
 auto* track=parsed.getTrack(0);int noteIndex=0;
 for(int i=0;i<track->getNumEvents();++i){const auto& e=track->getEventPointer(i)->message;
  if(e.isNoteOn()){require(noteIndex<int(original.notes.size()),"extra MIDI note");const auto& n=original.notes[size_t(noteIndex++)];require(e.getNoteNumber()==n.pitch && e.getVelocity()==n.velocity,"SMF note pitch/velocity");require(e.getTimeStamp()==std::lround(n.beat*960),"SMF note position");}
 }
 require(noteIndex==int(original.notes.size()),"SMF note count");require(track->getEndTime()==original.settings.bars*4*960,"SMF exact loop length");
juce::MemoryBlock state;p.getStateInformation(state);p.generate();require(p.snapshot().notes!=original.notes,"regenerate must change content");
 p.setStateInformation(state.getData(),int(state.getSize()));require(p.snapshot().notes==original.notes,"state must restore exact notes");require(p.snapshot().settings.seed==original.settings.seed,"state must restore seed");
 WeeProcessor restored;restored.setStateInformation(state.getData(),int(state.getSize()));require(restored.snapshot().notes==original.notes,"fresh instance state recall");
 require(std::abs(float(*restored.params.getRawParameterValue("gain"))-0.33f)<0.00001f,"automation parameter state recall");
 auto previewContent=restored.snapshot();previewContent.notes={{1.0,0.5,69,100}};restored.replace(std::move(previewContent));
 TestPlayHead host;host.position.setBpm(120.0);host.position.setPpqPosition(0.0);host.position.setIsPlaying(false);host.position.setTimeSignature(juce::AudioPlayHead::TimeSignature{4,4});restored.setPlayHead(&host);
 restored.setRateAndBufferSizeDetails(44100,256);restored.prepareToPlay(44100,256);restored.preview=true;double sum=0;for(int i=0;i<800;++i){midi.clear();restored.processBlock(buffer,midi);sum+=energy(buffer);}require(sum>1,"preview must render audible melody");
 restored.preview=false;for(int i=0;i<300;++i){midi.clear();restored.processBlock(buffer,midi);}require(energy(buffer)<0.000001,"stop must release generated voices");
 host.position.setIsPlaying(true);host.position.setBpm(140.0);midi.clear();restored.processBlock(buffer,midi);require(restored.tempo==140.0,"host tempo following");
 host.position.setTimeSignature(juce::AudioPlayHead::TimeSignature{3,4});midi.clear();restored.processBlock(buffer,midi);require(restored.unsupportedMeter,"unsupported meter warning");restored.setPlayHead(nullptr);
 std::cout<<"PASS: MIDI input piano, generation, state round-trip, preview audio and stop\n";
 }catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}return 0;}
