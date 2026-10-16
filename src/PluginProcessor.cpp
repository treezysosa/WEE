#include "Music/Editing.h"
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <bit>
namespace {
template<size_t N> juce::StringArray strings(const std::array<const char*,N>& names){juce::StringArray out;for(auto n:names)out.add(n);return out;}
}
juce::String WeeProcessor::layerId(int layer,const juce::String& name){return "layer"+juce::String(layer)+"_"+name;}
juce::AudioProcessorValueTreeState::ParameterLayout WeeProcessor::layout(){
 juce::AudioProcessorValueTreeState::ParameterLayout p;
 p.add(std::make_unique<juce::AudioParameterChoice>("style","Style",strings(wee::styleNames),1));
 p.add(std::make_unique<juce::AudioParameterInt>("key","Key",0,11,9));
 p.add(std::make_unique<juce::AudioParameterChoice>("scale","Scale",juce::StringArray{"Minor","Major"},0));
 p.add(std::make_unique<juce::AudioParameterChoice>("bars","Bars",juce::StringArray{"8","16"},0));
 p.add(std::make_unique<juce::AudioParameterFloat>("bpm","Preview BPM",40.f,240.f,120.f));
 p.add(std::make_unique<juce::AudioParameterBool>("sync","Host sync",true));
 p.add(std::make_unique<juce::AudioParameterFloat>("gain","Volume",0.f,1.f,0.8f));
 for(int i=0;i<8;++i){auto name=juce::String(wee::layerNames[size_t(i)])+" ";
  p.add(std::make_unique<juce::AudioParameterBool>(layerId(i,"enabled"),name+"Enabled",i==0));
  p.add(std::make_unique<juce::AudioParameterBool>(layerId(i,"mute"),name+"Mute",false));
  p.add(std::make_unique<juce::AudioParameterBool>(layerId(i,"solo"),name+"Solo",false));
  p.add(std::make_unique<juce::AudioParameterBool>(layerId(i,"lock"),name+"Lock notes",false));
  p.add(std::make_unique<juce::AudioParameterFloat>(layerId(i,"volume"),name+"Volume",0.f,1.f,i==0?0.8f:0.4f));
  p.add(std::make_unique<juce::AudioParameterFloat>(layerId(i,"pan"),name+"Pan",-1.f,1.f,0.f));
  p.add(std::make_unique<juce::AudioParameterChoice>(layerId(i,"instrument"),name+"Instrument",strings(wee::instrumentNames),wee::defaultInstruments[size_t(i)]));
 }
 for(const auto& spec:wee::effectParameters){
  if(spec.kind==1)p.add(std::make_unique<juce::AudioParameterBool>(spec.id,spec.label,spec.value>0));
  else if(spec.kind==2)p.add(std::make_unique<juce::AudioParameterInt>(spec.id,spec.label,int(spec.low),int(spec.high),int(spec.value)));
  else if(spec.kind==3)p.add(std::make_unique<juce::AudioParameterChoice>(spec.id,spec.label,juce::StringArray{"16-bit PCM","24-bit PCM","32-bit float"},int(spec.value)));
  else p.add(std::make_unique<juce::AudioParameterFloat>(spec.id,spec.label,spec.low,spec.high,spec.value));
 }
 p.add(std::make_unique<juce::AudioParameterChoice>("mood","Phrase mood",juce::StringArray{"Balanced","Reflective","Energetic"},0));
 return p;
}
WeeProcessor::WeeProcessor():AudioProcessor(BusesProperties().withOutput("Stereo",juce::AudioChannelSet::stereo(),true)),params(*this,nullptr,"WEE",layout()){
 for(int i=0;i<8;++i){auto& c=controls[size_t(i)];auto get=[&](const char* n){return params.getRawParameterValue(layerId(i,n));};
  c={get("enabled"),get("mute"),get("solo"),get("volume"),get("pan"),get("instrument"),get("lock")};
  auto& synth=engines[size_t(i)];for(int v=0;v<16;++v)synth.addVoice(new InstrumentVoice(c.instrument));synth.addSound(new PianoSound());synth.setMinimumRenderingSubdivisionSize(1,true);
 }
 bpmParam=params.getRawParameterValue("bpm");syncParam=params.getRawParameterValue("sync");gainParam=params.getRawParameterValue("gain");
 for(int i=0;i<10;++i)effectRefs[size_t(i)]=params.getRawParameterValue(wee::effectParameters[size_t(i)].id);
 for(auto* raw:getParameters())if(auto* parameter=dynamic_cast<juce::RangedAudioParameter*>(raw))params.addParameterListener(parameter->paramID,this);
 generate();
}
WeeProcessor::~WeeProcessor(){for(auto* raw:getParameters())if(auto* parameter=dynamic_cast<juce::RangedAudioParameter*>(raw))params.removeParameterListener(parameter->paramID,this);}
void WeeProcessor::parameterChanged(const juce::String& id,float){if(id!="gain"&&!id.startsWith("wav_")&&id!="flip_export"){++changeToken;auditionMode=0;}}
double WeeProcessor::exportTempo()const{return syncParam->load()>0.5f&&hostTempoAvailable?tempo.load():double(bpmParam->load());}
double WeeProcessor::getTailLengthSeconds()const{if(!effectRefs[0]||effectRefs[0]->load()<0.5f)return 3;double delay=effectRefs[2]->load()>0?60.0/std::clamp(exportTempo(),20.0,999.0)*11:0;return std::min(30.0,3+std::max(effectRefs[1]->load()>0?2.5:0.0,delay));}
wee::EffectSettings WeeProcessor::effectSettings()const{wee::EffectSettings s;s.enabled=effectRefs[0]->load()>0.5f;s.reverb=effectRefs[1]->load();s.delay=effectRefs[2]->load();s.chorus=effectRefs[3]->load();s.saturation=effectRefs[4]->load();s.lowpass=effectRefs[5]->load();s.highpass=effectRefs[6]->load();s.width=effectRefs[7]->load();s.wow=effectRefs[8]->load();s.vinyl=effectRefs[9]->load();return s;}
wee::FlipSettings WeeProcessor::flipSettings()const{auto get=[&](const char*id){return params.getRawParameterValue(id)->load();};wee::FlipSettings s;s.half=get("flip_half")>0.5f;s.reverse=get("flip_reverse")>0.5f;s.transpose=int(get("flip_pitch"));s.tape=get("flip_tape");s.wow=get("flip_wow");s.lowpass=get("flip_lowpass");s.reverb=get("flip_reverb");s.vintage=get("flip_vintage");return s;}
void WeeProcessor::publishAudition(std::unique_ptr<WeeAudition> pair){if(!pair||pair->original.size()==0||pair->original.size()!=pair->flipped.size())return;std::unique_ptr<WeeAudition> old;{const juce::SpinLock::ScopedLockType lock(auditionLock);if(pair->token!=changeToken.load())return;old=std::move(audition);audition=std::move(pair);clipTempo=audition->bpm;clipToken=audition->token;}}

wee::Arrangement WeeProcessor::snapshotArrangement(){const juce::SpinLock::ScopedLockType l(contentLock);return content;}
wee::Composition WeeProcessor::snapshotLayer(int i){const juce::SpinLock::ScopedLockType l(contentLock);return content.layers[size_t(std::clamp(i,0,7))];}
wee::Composition WeeProcessor::snapshot(){const juce::SpinLock::ScopedLockType l(contentLock);return content.layers[0];}
void WeeProcessor::replaceArrangement(wee::Arrangement a){
 std::array<wee::LayerSchedule,8> prepared;for(int i=0;i<8;++i)prepared[size_t(i)]=wee::makeSchedule(a.layers[size_t(i)]);
 ++changeToken;auditionMode=0;
 {const juce::SpinLock::ScopedLockType l(contentLock);std::swap(content,a);schedules.swap(prepared);for(auto& r:revision)++r;}
 updateHostDisplay(juce::AudioProcessorListener::ChangeDetails{}.withNonParameterStateChanged(true));
}
void WeeProcessor::replace(wee::Composition c){auto a=wee::generateArrangement(c.settings);a.layers[0]=std::move(c);replaceArrangement(std::move(a));}
bool WeeProcessor::generate(){
 if(unsupportedMeter.load()){generationStatus=3;return false;}
 wee::Settings s;s.key=int(*params.getRawParameterValue("key"));s.scale=int(*params.getRawParameterValue("scale"));s.style=int(*params.getRawParameterValue("style"));s.bars=*params.getRawParameterValue("bars")>0?16:8;s.bpm=bpmParam->load();s.mood=int(*params.getRawParameterValue("mood"));
 auto old=snapshotArrangement();bool anyLock=false;for(auto& c:controls)anyLock=anyLock||c.locked->load()>0.5f;
 if(anyLock&&!wee::sameHarmony(s,old.context)){generationStatus=1;return false;}
 s.seed=seed.fetch_add(1);auto a=wee::generateArrangement(s);for(int i=0;i<8;++i)if(controls[size_t(i)].locked->load()>0.5f)a.layers[size_t(i)]=old.layers[size_t(i)];
 replaceArrangement(std::move(a));generationStatus=0;return true;
}
bool WeeProcessor::regenerateLayer(int i){if(unsupportedMeter.load()){generationStatus=3;return false;}if(i<0||i>=8)return false;if(controls[size_t(i)].locked->load()>0.5f){generationStatus=2;return false;}
 auto a=snapshotArrangement();auto s=a.context;s.seed=seed.fetch_add(1);auto layer=wee::generateLayer(s,i);auto prepared=wee::makeSchedule(layer);
 {const juce::SpinLock::ScopedLockType l(contentLock);std::swap(content.layers[size_t(i)],layer);std::swap(schedules[size_t(i)],prepared);++revision[size_t(i)];}
 ++changeToken;auditionMode=0;generationStatus=0;updateHostDisplay(juce::AudioProcessorListener::ChangeDetails{}.withNonParameterStateChanged(true));return true;
}
std::array<bool,8> WeeProcessor::audibleLayers() const {
 bool solo=false;for(auto& c:controls)solo|=c.enabled->load()>0.5f&&c.solo->load()>0.5f;
 std::array<bool,8> active{};for(int i=0;i<8;++i){auto& c=controls[size_t(i)];active[size_t(i)]=c.enabled->load()>0.5f&&c.mute->load()<0.5f&&(!solo||c.solo->load()>0.5f);}return active;
}
void WeeProcessor::prepareToPlay(double sr,int block){
 for(auto& peak:meterPeak)peak.store(0);
 preparedRate=sr;++changeToken;auditionMode=0;effects.prepare(sr);auditionBlend.reset(sr,0.02);auditionBlend.setCurrentAndTargetValue(0);auditionGate.reset(sr,0.02);auditionGate.setCurrentAndTargetValue(0);auditionBeat=0;
 preparedBlock=std::max(1,block);reportedBlock=preparedBlock;for(int i=0;i<8;++i){engines[size_t(i)].setCurrentPlaybackSampleRate(sr);audio[size_t(i)].setSize(1,preparedBlock);events[size_t(i)].ensureSize(65536);left[size_t(i)].reset(sr,0.02);right[size_t(i)].reset(sr,0.02);left[size_t(i)].setCurrentAndTargetValue(0);right[size_t(i)].setCurrentAndTargetValue(0);}
 gain.reset(sr,0.02);gain.setCurrentAndTargetValue(gainParam->load());previewBeat=0;wasPlaying=wasHost=false;previousAudible.fill(false);previousInstrument.fill(-1);
}
void WeeProcessor::processBlock(juce::AudioBuffer<float>& output,juce::MidiBuffer& incoming){
 juce::ScopedNoDenormals dn;output.clear();double sr=getSampleRate();if(!std::isfinite(sr)||sr<=0||output.getNumSamples()==0){incoming.clear();return;}
 bool host=false,bad=false,tempoKnown=false;double beat=previewBeat,bpm=bpmParam->load();
 if(auto* ph=getPlayHead())if(auto pos=ph->getPosition()){
  if(syncParam->load()>0.5f){host=pos->getIsPlaying();if(auto t=pos->getBpm())if(std::isfinite(*t)&&*t>0){bpm=juce::jlimit(20.0,999.0,*t);tempoKnown=true;}if(host)if(auto p=pos->getPpqPosition())if(std::isfinite(*p)&&std::abs(*p)<1.0e9)beat=*p;}
  if(auto ts=pos->getTimeSignature())bad=ts->numerator!=4||ts->denominator!=4;
 }
 hostTempoAvailable=tempoKnown;tempo=bpm;if(clipToken.load()!=0&&std::abs(clipTempo.load()-bpm)>0.001){clipToken=0;auditionMode=0;}unsupportedMeter=bad;bool playing=(host||preview.load())&&!bad;double step=bpm/(60*sr);
 bool discontinuity=host&&(!wasHost||std::abs(beat-expectedHostBeat)>step*2);
 auto active=audibleLayers();
 for(int i=0;i<8;++i){size_t k=size_t(i);auto& c=controls[k];int type=int(c.instrument->load());auto rev=revision[k].load();
  if(discontinuity||renderedRevision[k]!=rev||previousInstrument[k]!=type)engines[k].allNotesOff(0,false);
  else if((wasPlaying&&!playing)||(previousAudible[k]&&!active[k])||bad)engines[k].allNotesOff(0,true);
  renderedRevision[k]=rev;previousInstrument[k]=type;previousAudible[k]=active[k];
  float volume=active[k]?c.volume->load():0,pan=c.pan->load();left[k].setTargetValue(volume*std::sqrt((1-pan)*0.5f));right[k].setTargetValue(volume*std::sqrt((1+pan)*0.5f));
 }
 int bars=playbackBars;
 for(int start=0;start<output.getNumSamples();start+=preparedBlock){int count=std::min(preparedBlock,output.getNumSamples()-start);for(auto& e:events)e.clear();
  if(active[0])events[0].addEvents(incoming,start,count,-start);
  if(playing){juce::SpinLock::ScopedTryLockType lock(contentLock);if(lock.isLocked()){
   bars=playbackBars=content.context.bars;double loop=bars*4,begin=beat+start*step,end=begin+count*step;
   for(int i=0;i<8;++i)if(active[size_t(i)])for(int off=1;off>=0;--off){const auto& stream=off?schedules[size_t(i)].off:schedules[size_t(i)].on;
    wee::scheduleRange(stream,begin,end,loop,[&](const wee::ScheduledEvent& e,double at){int sample=juce::jlimit(0,count-1,int(std::lround((at-begin)/step)));events[size_t(i)].addEvent(off?juce::MidiMessage::noteOff(i+1,e.pitch):juce::MidiMessage::noteOn(i+1,e.pitch,(juce::uint8)e.velocity),sample);});
   }
  }else for(auto& synth:engines)synth.allNotesOff(0,true);}
  for(int i=0;i<8;++i){size_t k=size_t(i);bool voices=false;for(int voice=0;voice<engines[k].getNumVoices();++voice)voices|=engines[k].getVoice(voice)->isVoiceActive();
   if(events[k].isEmpty()&&!voices){left[k].skip(count);right[k].skip(count);continue;}
   audio[k].clear();engines[k].renderNextBlock(audio[k],events[k],0,count);const float* samples=audio[k].getReadPointer(0);
   for(int t=0;t<count;++t){output.addSample(0,start+t,samples[t]*left[k].getNextValue());output.addSample(1,start+t,samples[t]*right[k].getNextValue());}
  }
 }
 previewBeat=playing?std::fmod(beat+output.getNumSamples()*step,bars*4):0;expectedHostBeat=beat+output.getNumSamples()*step;wasPlaying=playing;wasHost=host;
 effects.set(effectSettings());
 for(int start=0;start<output.getNumSamples();start+=preparedBlock){int count=std::min(preparedBlock,output.getNumSamples()-start);effects.process(output.getWritePointer(0,start),output.getWritePointer(1,start),count,bpm);}
 int mode=auditionMode.load();if(mode>0&&!auditionReady()){auditionMode=0;mode=0;}if(mode>0&&auditionReady()){
  juce::SpinLock::ScopedTryLockType lock(auditionLock);
  if(lock.isLocked()&&audition&&audition->token==changeToken.load()){
   if(std::abs(audition->bpm-bpm)>0.001){auditionMode=0;clipToken=0;}
   else{auditionBlend.setTargetValue(mode==2?1.f:0.f);auditionGate.setTargetValue(playing?1.f:0.f);double cacheBeat=playing?beat:auditionBeat;auditionBeat=cacheBeat+output.getNumSamples()*step;size_t n=audition->original.size();
    for(int t=0;t<output.getNumSamples();++t){float blend=auditionBlend.getNextValue(),gate=auditionGate.getNextValue();double position=std::fmod((cacheBeat+t*step)/(audition->bars*4),1.0);if(position<0)position+=1;position*=double(n);size_t a=size_t(position)%n,b=(a+1)%n;float f=float(position-double(a));for(int c=0;c<2;++c){const auto& original=audition->original.channels[size_t(c)];const auto& flipped=audition->flipped.channels[size_t(c)];float dry=original[a]*(1-f)+original[b]*f,wet=flipped[a]*(1-f)+flipped[b]*f;output.setSample(c,t,(dry*(1-blend)+wet*blend)*gate);}}
   }
  }else output.clear();
 }
 gain.setTargetValue(gainParam->load());std::array<float,2> peaks{};for(int t=0;t<output.getNumSamples();++t){float g=gain.getNextValue();for(int ch=0;ch<2;++ch){auto& value=output.getWritePointer(ch)[t];value*=g;peaks[size_t(ch)]=std::max(peaks[size_t(ch)],std::abs(value));}}
 for(int ch=0;ch<2;++ch){auto& peak=meterPeak[size_t(ch)];float previous=peak.load();while(previous<peaks[size_t(ch)]&&!peak.compare_exchange_weak(previous,peaks[size_t(ch)])){} }incoming.clear();
}
namespace {
void writeSettings(juce::ValueTree& v,const wee::Settings&s){v.setProperty("bars",s.bars,nullptr);v.setProperty("key",s.key,nullptr);v.setProperty("scale",s.scale,nullptr);v.setProperty("style",s.style,nullptr);v.setProperty("seed",juce::int64(s.seed),nullptr);v.setProperty("bpm",s.bpm,nullptr);v.setProperty("mood",s.mood,nullptr);}
void writeNotes(juce::ValueTree& v,const wee::Composition&c){v.setProperty("seed",juce::int64(c.settings.seed),nullptr);for(auto& n:c.notes){juce::ValueTree note("Note");note.setProperty("beat",n.beat,nullptr);note.setProperty("length",n.length,nullptr);note.setProperty("beatBits",juce::String::toHexString(juce::int64(std::bit_cast<uint64_t>(n.beat))),nullptr);note.setProperty("lengthBits",juce::String::toHexString(juce::int64(std::bit_cast<uint64_t>(n.length))),nullptr);note.setProperty("pitch",n.pitch,nullptr);note.setProperty("velocity",n.velocity,nullptr);v.appendChild(note,nullptr);}}
bool readNotes(const juce::ValueTree&v,wee::Composition&c){if(v.getNumChildren()>2048)return false;for(auto n:v){wee::Note note{double(n["beat"]),double(n["length"]),int(n["pitch"]),int(n["velocity"])};if(n.hasProperty("beatBits"))note.beat=std::bit_cast<double>(uint64_t(n["beatBits"].toString().getHexValue64()));if(n.hasProperty("lengthBits"))note.length=std::bit_cast<double>(uint64_t(n["lengthBits"].toString().getHexValue64()));if(!n.hasType("Note")||!std::isfinite(note.beat)||!std::isfinite(note.length)||note.beat<0||note.length<=0||note.beat+note.length>c.settings.bars*4+1.0e-8||note.pitch<0||note.pitch>127||note.velocity<1||note.velocity>127)return false;c.notes.push_back(note);}wee::sort(c);return true;}
}
void WeeProcessor::getContentState(juce::MemoryBlock&out){auto tree=params.copyState();tree.setProperty("version",4,nullptr);tree.setProperty("nextSeed",juce::int64(seed.load()),nullptr);auto a=snapshotArrangement();juce::ValueTree song("Arrangement");writeSettings(song,a.context);for(int i=0;i<8;++i){juce::ValueTree layer("Layer");layer.setProperty("index",i,nullptr);writeNotes(layer,a.layers[size_t(i)]);song.appendChild(layer,nullptr);}tree.appendChild(song,nullptr);auto xml=tree.createXml();copyXmlToBinary(*xml,out);}
bool WeeProcessor::restoreContent(const void*data,int size){
 if(!data||size<=0||size>4*1024*1024)return false;auto xml=getXmlFromBinary(data,size);if(!xml)return false;auto tree=juce::ValueTree::fromXml(*xml);if(!tree.hasType("WEE"))return false;int version=int(tree["version"]);if(version!=1&&version!=2&&version!=3&&version!=4)return false;
 // Build a full parameter tree first; missing Phase 2 controls get their defaults.
 juce::ValueTree restored("WEE");for(auto* raw:getParameters())if(auto* parameter=dynamic_cast<juce::RangedAudioParameter*>(raw)){
  double value=parameter->convertFrom0to1(parameter->getDefaultValue());auto input=tree.getChildWithProperty("id",parameter->paramID);if(input.isValid())value=double(input["value"]);
  auto range=parameter->getNormalisableRange();if(!std::isfinite(value)||value<range.start||value>range.end)return false;juce::ValueTree v("PARAM");v.setProperty("id",parameter->paramID,nullptr);v.setProperty("value",range.snapToLegalValue(float(value)),nullptr);restored.appendChild(v,nullptr);
 }
 auto value=[&](const char* id){return double(restored.getChildWithProperty("id",id)["value"]);};wee::Settings settings;settings.key=int(value("key"));settings.scale=int(value("scale"));settings.style=int(value("style"));settings.bars=value("bars")>0?16:8;settings.bpm=value("bpm");settings.mood=int(value("mood"));
 wee::Arrangement a;
 if(version==1){auto song=tree.getChildWithName("Song");if(!song.isValid())return false;settings.bars=int(song["bars"]);settings.seed=uint32_t(juce::int64(song["seed"]));settings.bpm=double(song["bpm"]);if((settings.bars!=8&&settings.bars!=16)||!std::isfinite(settings.bpm)||settings.bpm<20||settings.bpm>999)return false;
  a=wee::generateArrangement(settings);a.context=settings;a.layers[0]={settings,{}};if(!readNotes(song,a.layers[0]))return false;
 }else{auto song=tree.getChildWithName("Arrangement");if(!song.isValid()||song.getNumChildren()!=8)return false;settings.bars=int(song["bars"]);settings.key=int(song["key"]);settings.scale=int(song["scale"]);settings.style=int(song["style"]);settings.mood=song.hasProperty("mood")?int(song["mood"]):0;settings.seed=uint32_t(juce::int64(song["seed"]));settings.bpm=double(song["bpm"]);
  if((settings.bars!=8&&settings.bars!=16)||settings.key<0||settings.key>11||settings.scale<0||settings.scale>1||settings.style<0||settings.style>8||settings.mood<0||settings.mood>2||!std::isfinite(settings.bpm)||settings.bpm<20||settings.bpm>999)return false;
  a.context=settings;std::array<bool,8> found{};for(auto layer:song){int i=int(layer["index"]);if(!layer.hasType("Layer")||i<0||i>=8||found[size_t(i)])return false;found[size_t(i)]=true;a.layers[size_t(i)].settings=settings;a.layers[size_t(i)].settings.seed=uint32_t(juce::int64(layer["seed"]));if(!readNotes(layer,a.layers[size_t(i)]))return false;}
 }
 params.replaceState(restored);seed=version>=2?uint32_t(juce::int64(tree["nextSeed"])):settings.seed+1;replaceArrangement(std::move(a));generationStatus=0;return true;
}
juce::AudioProcessorEditor* WeeProcessor::createEditor(){return new WeeEditor(*this);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new WeeProcessor();}

bool WeeProcessor::editLayer(int index,const wee::Composition& edited){
 if(index<0||index>=8||unsupportedMeter||!wee::validNotes(edited))return false;
 if(controls[size_t(index)].locked->load()>0.5f){generationStatus=2;return false;}
 auto layer=edited;wee::sort(layer);auto prepared=wee::makeSchedule(layer);size_t i=size_t(index);
 {const juce::SpinLock::ScopedLockType l(contentLock);if(!wee::sameHarmony(content.context,edited.settings)||content.layers[i].notes==edited.notes)return false;
 std::swap(content.layers[i],layer);std::swap(schedules[i],prepared);++revision[i];}
 ++changeToken;auditionMode=0;generationStatus=0;updateHostDisplay(juce::AudioProcessorListener::ChangeDetails{}.withNonParameterStateChanged(true));return true;
}

void WeeProcessor::trimHistory(std::vector<juce::MemoryBlock>& entries){
 size_t total=0;for(auto& b:entries)total+=b.getSize();
 while(!entries.empty()&&(entries.size()>8||total>2*1024*1024)){total-=entries.front().getSize();entries.erase(entries.begin());}
}
void WeeProcessor::remember(juce::MemoryBlock before){const juce::ScopedLock lock(historyLock);past.push_back(std::move(before));trimHistory(past);future.clear();}
bool WeeProcessor::canUndo()const{const juce::ScopedLock lock(historyLock);return !past.empty();}
bool WeeProcessor::canRedo()const{const juce::ScopedLock lock(historyLock);return !future.empty();}
bool WeeProcessor::travelHistory(bool redo){
 const juce::ScopedLock lock(historyLock);auto& from=redo?future:past;auto& to=redo?past:future;if(from.empty())return false;
 juce::MemoryBlock now;getContentState(now);auto& wanted=from.back();if(!restoreContent(wanted.getData(),int(wanted.getSize())))return false;
 from.pop_back();to.push_back(std::move(now));trimHistory(to);return true;
}
bool WeeProcessor::loadPreset(const juce::MemoryBlock& bytes){juce::MemoryBlock before;getContentState(before);if(!restoreContent(bytes.getData(),int(bytes.getSize())))return false;remember(std::move(before));return true;}
void WeeProcessor::getStateInformation(juce::MemoryBlock& out){
 juce::MemoryBlock current;getContentState(current);auto xml=getXmlFromBinary(current.getData(),int(current.getSize()));auto tree=juce::ValueTree::fromXml(*xml);
 juce::ValueTree history("History");const juce::ScopedLock lock(historyLock);
 for(int direction=0;direction<2;++direction){juce::ValueTree list(direction==0?"Past":"Future");for(auto& bytes:direction==0?past:future){juce::ValueTree entry("Entry");entry.setProperty("data",bytes.toBase64Encoding(),nullptr);list.appendChild(entry,nullptr);}history.appendChild(list,nullptr);}
 tree.appendChild(history,nullptr);auto full=tree.createXml();copyXmlToBinary(*full,out);
}
void WeeProcessor::setStateInformation(const void* data,int size){
 if(!data||size<=0||size>12*1024*1024)return;auto xml=getXmlFromBinary(data,size);if(!xml)return;auto tree=juce::ValueTree::fromXml(*xml);
 auto history=tree.getChildWithName("History");tree.removeChild(history,nullptr);auto contentXml=tree.createXml();juce::MemoryBlock current;copyXmlToBinary(*contentXml,current);
 if(!restoreContent(current.getData(),int(current.getSize())))return;
 const juce::ScopedLock lock(historyLock);past.clear();future.clear();
 for(int direction=0;direction<2;++direction){auto list=history.getChildWithName(direction==0?"Past":"Future");if(list.getNumChildren()>8)continue;auto& entries=direction==0?past:future;
 for(auto entry:list){juce::MemoryBlock bytes;if(!entry.hasType("Entry")||entry["data"].toString().length()>3*1024*1024||!bytes.fromBase64Encoding(entry["data"].toString()))continue;
 auto inner=getXmlFromBinary(bytes.getData(),int(bytes.getSize()));if(!inner||!inner->hasTagName("WEE")||inner->getChildByName("History"))continue;entries.push_back(std::move(bytes));}trimHistory(entries);}
}

juce::String WeeProcessor::diagnosticReport(){
 auto a=snapshotArrangement();auto active=audibleLayers();juce::String report="WEE " JucePlugin_VersionString " — local diagnostic report\n";
 report+="Rate: "+juce::String(exportRate(),1)+" Hz; maximum prepared block: "+juce::String(reportedBlock.load())+" samples\n";
 report+="Tempo: "+juce::String(exportTempo(),3)+" BPM; host sync: "+juce::String(syncParam->load()>0.5f?"on":"off")+"\n";
 report+="Meter: "+juce::String(unsupportedMeter.load()?"unsupported (requires 4/4)":"4/4 or not supplied by host")+"\n";
 report+="Bars: "+juce::String(a.context.bars)+"; key class: "+juce::String(a.context.key)+"; scale: "+juce::String(a.context.scale==0?"minor":"major")+"; style: "+wee::styleNames[size_t(a.context.style)]+"\n";
 for(int i=0;i<8;++i)report+=juce::String(wee::layerNames[size_t(i)])+": "+juce::String(int(a.layers[size_t(i)].notes.size()))+" notes; seed "+juce::String(a.layers[size_t(i)].settings.seed)+"; audible "+juce::String(active[size_t(i)]?"yes":"no")+"\n";
 report+="No user paths or audio are collected. This file stays on your computer.\nFL Studio interactive compatibility is not certified by the automated Windows build.\nManual test checklist: https://github.com/treezysosa/WEE/blob/wee/phase-5/docs/FL-STUDIO-SETUP.md\n";return report;
}
