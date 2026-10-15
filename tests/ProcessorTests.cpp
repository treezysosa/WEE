#include "../src/PluginProcessor.h"
#include "../src/Export/Midi.h"
#include "../src/PluginEditor.h"
#include <iostream>
#include <stdexcept>
static void require(bool b,const char* msg){if(!b)throw std::runtime_error(msg);}
static void set(WeeProcessor&p,const juce::String&id,float value){auto* param=p.params.getParameter(id);require(param!=nullptr,"parameter exists");param->setValueNotifyingHost(param->convertTo0to1(value));}
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

 // Version-1 project migration preserves exact notes and keeps new layers disabled.
 auto legacyTree=p.params.copyState();legacyTree.setProperty("version",1,nullptr);juce::ValueTree legacySong("Song");legacySong.setProperty("bars",original.settings.bars,nullptr);legacySong.setProperty("seed",juce::int64(original.settings.seed),nullptr);legacySong.setProperty("bpm",120.0,nullptr);
 for(auto&n:original.notes){juce::ValueTree v("Note");v.setProperty("beat",n.beat,nullptr);v.setProperty("length",n.length,nullptr);v.setProperty("pitch",n.pitch,nullptr);v.setProperty("velocity",n.velocity,nullptr);legacySong.appendChild(v,nullptr);}
 for(int i=legacyTree.getNumChildren()-1;i>=0;--i)if(legacyTree.getChild(i)["id"].toString().startsWith("layer"))legacyTree.removeChild(i,nullptr);
 legacyTree.appendChild(legacySong,nullptr);juce::MemoryBlock legacy;auto legacyXml=legacyTree.createXml();juce::AudioProcessor::copyXmlToBinary(*legacyXml,legacy);
 WeeProcessor migrated;migrated.setStateInformation(legacy.getData(),int(legacy.getSize()));require(migrated.snapshot().notes==original.notes,"Phase 1 exact note migration");for(int i=1;i<8;++i)require(!migrated.audibleLayers()[size_t(i)],"new layers disabled in old projects");
 WeeProcessor layers;auto arrangement=layers.snapshotArrangement();set(layers,WeeProcessor::layerId(0,"lock"),1);require(layers.generate(),"generate with locked lead");require(layers.snapshot().notes==arrangement.layers[0].notes,"locked lead preserved");
 require(!layers.regenerateLayer(0),"locked layer rejects regeneration");set(layers,"key",2);require(!layers.generate(),"harmonic change blocked with locks");require(layers.snapshotArrangement().context.key==9,"blocked generate preserves harmony");set(layers,"key",9);
 auto before=layers.snapshotArrangement();require(layers.regenerateLayer(3),"unlocked piano regeneration");auto after=layers.snapshotArrangement();for(int i=0;i<8;++i)if(i!=3)require(before.layers[size_t(i)].notes==after.layers[size_t(i)].notes,"per-layer regeneration isolation");require(before.layers[3].notes!=after.layers[3].notes,"regenerated piano varies");
 set(layers,WeeProcessor::layerId(2,"enabled"),1);set(layers,WeeProcessor::layerId(2,"solo"),1);auto audible=layers.audibleLayers();require(audible[2]&&!audible[0],"solo isolation");set(layers,WeeProcessor::layerId(2,"mute"),1);require(!layers.audibleLayers()[2],"mute wins over solo");
 juce::MemoryBlock layerState;layers.getStateInformation(layerState);WeeProcessor recalled;recalled.setStateInformation(layerState.getData(),int(layerState.getSize()));auto recalledSong=recalled.snapshotArrangement();for(int i=0;i<8;++i)require(recalledSong.layers[size_t(i)].notes==after.layers[size_t(i)].notes,"all layer note state recall");require(*recalled.params.getRawParameterValue(WeeProcessor::layerId(0,"lock"))==1,"lock recalled");require(*recalled.params.getRawParameterValue(WeeProcessor::layerId(2,"solo"))==1,"solo recalled");
 auto multi=wee::midi(after,std::array<bool,8>{true,true,true,true,true,true,true,true});juce::MemoryInputStream multiStream(multi.data(),multi.size(),false);juce::MidiFile multiParsed;require(multiParsed.readFrom(multiStream)&&multiParsed.getNumTracks()==9,"JUCE format-1 multi-layer import");
 for(int layer=0;layer<8;++layer){int ons=0;const auto*t=multiParsed.getTrack(layer+1);for(int i=0;i<t->getNumEvents();++i){auto&msg=t->getEventPointer(i)->message;if(msg.isNoteOn()){require(msg.getChannel()==layer+1,"separate layer MIDI channel");++ons;}}require(ons==int(after.layers[size_t(layer)].notes.size()),"layer MIDI count");require(t->getEndTime()==after.context.bars*4*960,"layer MIDI duration");}
 // Every instrument produces a distinct, finite waveform at both supported test rates.
 std::array<double,8> signatures{};for(double rate:{44100.0,48000.0})for(int kind=0;kind<8;++kind){WeeProcessor instrument;set(instrument,WeeProcessor::layerId(0,"instrument"),float(kind));instrument.setRateAndBufferSizeDetails(rate,256);instrument.prepareToPlay(rate,256);double sumEnergy=0,signature=0;for(int block=0;block<80;++block){midi.clear();if(block==0)midi.addEvent(juce::MidiMessage::noteOn(1,60,(juce::uint8)100),0);instrument.processBlock(buffer,midi);sumEnergy+=energy(buffer);for(int i=0;i<256;++i)signature+=buffer.getSample(0,i)*(1+(i%17));}require(sumEnergy>0.01,"instrument audio");if(rate==48000)signatures[size_t(kind)]=signature;}
 for(int i=0;i<8;++i)for(int j=0;j<i;++j)require(std::abs(signatures[size_t(i)]-signatures[size_t(j)])>0.0001,"distinct instrument timbres");
 // Hard-left pan routes audio exclusively to the left output after smoothing.
 WeeProcessor pan;set(pan,WeeProcessor::layerId(0,"pan"),-1);pan.setRateAndBufferSizeDetails(48000,256);pan.prepareToPlay(48000,256);midi.clear();midi.addEvent(juce::MidiMessage::noteOn(1,60,(juce::uint8)100),0);pan.processBlock(buffer,midi);double le=0,re=0;for(int i=0;i<256;++i){le+=std::abs(buffer.getSample(0,i));re+=std::abs(buffer.getSample(1,i));}require(le>0&&re==0,"equal-power layer pan");

 // All-layer playback and mixer muting exercise the actual rendering path, not just its masks.
 WeeProcessor mix;for(int i=0;i<8;++i)set(mix,WeeProcessor::layerId(i,"enabled"),1);mix.setRateAndBufferSizeDetails(48000,256);mix.prepareToPlay(48000,256);mix.preview=true;double mixedEnergy=0;for(int block=0;block<600;++block){midi.clear();mix.processBlock(buffer,midi);mixedEnergy+=energy(buffer);}require(mixedEnergy>1,"eight-layer audio render");
 for(int i=0;i<8;++i)set(mix,WeeProcessor::layerId(i,"mute"),1);for(int block=0;block<30;++block){midi.clear();mix.processBlock(buffer,midi);}require(energy(buffer)<1.0e-10,"all layer mutes silence actual audio");
 // A host may deliver larger blocks than advertised; the preallocated chunk path must work.
 juce::AudioBuffer<float> oversized(2,1024);midi.clear();pan.processBlock(oversized,midi);require(energy(oversized)>0,"oversized block chunk rendering");
 // Bad state is rejected atomically, preserving unsaved generated content.
 auto good=layers.snapshotArrangement();auto badXml=juce::AudioProcessor::getXmlFromBinary(layerState.getData(),int(layerState.getSize()));auto malformed=juce::ValueTree::fromXml(*badXml);malformed.getChildWithName("Arrangement").getChild(0).getChild(0).setProperty("pitch",999,nullptr);auto invalidXml=malformed.createXml();juce::MemoryBlock invalid;juce::AudioProcessor::copyXmlToBinary(*invalidXml,invalid);layers.setStateInformation(invalid.getData(),int(invalid.getSize()));require(layers.snapshot().notes==good.layers[0].notes,"invalid state preserves content");
 // Construct, resize and destroy the editor at its compact and large sizes.
 {std::unique_ptr<juce::AudioProcessorEditor> editor(layers.createEditor());editor->setSize(850,650);editor->setSize(1600,1250);}
 std::cout<<"PASS: MIDI input piano, generation, state round-trip, preview audio and stop\n";
 }catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}return 0;}
