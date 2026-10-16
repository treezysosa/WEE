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
wee::Composition WeeProcessor::snapshot(){const juce::SpinLock::ScopedLockType l(contentLock);return content.layers[0];}
void WeeProcessor::replaceArrangement(wee::Arrangement a){
 ++changeToken;auditionMode=0;
 {const juce::SpinLock::ScopedLockType l(contentLock);std::swap(content,a);for(auto& r:revision)++r;}
 updateHostDisplay(juce::AudioProcessorListener::ChangeDetails{}.withNonParameterStateChanged(true));
}
void WeeProcessor::replace(wee::Composition c){auto a=wee::generateArrangement(c.settings);a.layers[0]=std::move(c);replaceArrangement(std::move(a));}
bool WeeProcessor::generate(){
 wee::Settings s;s.key=int(*params.getRawParameterValue("key"));s.scale=int(*params.getRawParameterValue("scale"));s.style=int(*params.getRawParameterValue("style"));s.bars=*params.getRawParameterValue("bars")>0?16:8;s.bpm=bpmParam->load();
 auto old=snapshotArrangement();bool anyLock=false;for(auto& c:controls)anyLock=anyLock||c.locked->load()>0.5f;
 if(anyLock&&!wee::sameHarmony(s,old.context)){generationStatus=1;return false;}
 s.seed=seed.fetch_add(1);auto a=wee::generateArrangement(s);for(int i=0;i<8;++i)if(controls[size_t(i)].locked->load()>0.5f)a.layers[size_t(i)]=old.layers[size_t(i)];
 replaceArrangement(std::move(a));generationStatus=0;return true;
}
bool WeeProcessor::regenerateLayer(int i){if(i<0||i>=8)return false;if(controls[size_t(i)].locked->load()>0.5f){generationStatus=2;return false;}
 auto a=snapshotArrangement();auto s=a.context;s.seed=seed.fetch_add(1);auto layer=wee::generateLayer(s,i);
 {const juce::SpinLock::ScopedLockType l(contentLock);content.layers[size_t(i)]=std::move(layer);++revision[size_t(i)];}
 ++changeToken;auditionMode=0;generationStatus=0;updateHostDisplay(juce::AudioProcessorListener::ChangeDetails{}.withNonParameterStateChanged(true));return true;
}
std::array<bool,8> WeeProcessor::audibleLayers() const {
 bool solo=false;for(auto& c:controls)solo|=c.enabled->load()>0.5f&&c.solo->load()>0.5f;
 std::array<bool,8> active{};for(int i=0;i<8;++i){auto& c=controls[size_t(i)];active[size_t(i)]=c.enabled->load()>0.5f&&c.mute->load()<0.5f&&(!solo||c.solo->load()>0.5f);}return active;
}
void WeeProcessor::prepareToPlay(double sr,int block){
 preparedRate=sr;++changeToken;auditionMode=0;effects.prepare(sr);auditionBlend.reset(sr,0.02);auditionBlend.setCurrentAndTargetValue(0);auditionGate.reset(sr,0.02);auditionGate.setCurrentAndTargetValue(0);auditionBeat=0;
 preparedBlock=std::max(1,block);for(int i=0;i<8;++i){engines[size_t(i)].setCurrentPlaybackSampleRate(sr);audio[size_t(i)].setSize(1,preparedBlock);events[size_t(i)].ensureSize(65536);left[size_t(i)].reset(sr,0.02);right[size_t(i)].reset(sr,0.02);left[size_t(i)].setCurrentAndTargetValue(0);right[size_t(i)].setCurrentAndTargetValue(0);}
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
   for(int i=0;i<8;++i)if(active[size_t(i)])for(int off=1;off>=0;--off)for(const auto& n:content.layers[size_t(i)].notes){
    double event=n.beat+(off?n.length:0),at=event+std::ceil((begin-event)/loop)*loop;
    while(at<end){int sample=juce::jlimit(0,count-1,int(std::lround((at-begin)/step)));events[size_t(i)].addEvent(off?juce::MidiMessage::noteOff(i+1,n.pitch):juce::MidiMessage::noteOn(i+1,n.pitch,(juce::uint8)n.velocity),sample);at+=loop;}
   }
  }else for(auto& synth:engines)synth.allNotesOff(0,true);}
  for(int i=0;i<8;++i){size_t k=size_t(i);audio[k].clear();engines[k].renderNextBlock(audio[k],events[k],0,count);const float* samples=audio[k].getReadPointer(0);
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
 gain.setTargetValue(gainParam->load());for(int t=0;t<output.getNumSamples();++t){float g=gain.getNextValue();for(int ch=0;ch<output.getNumChannels();++ch)output.getWritePointer(ch)[t]*=g;}incoming.clear();
}
namespace {
void writeSettings(juce::ValueTree& v,const wee::Settings&s){v.setProperty("bars",s.bars,nullptr);v.setProperty("key",s.key,nullptr);v.setProperty("scale",s.scale,nullptr);v.setProperty("style",s.style,nullptr);v.setProperty("seed",juce::int64(s.seed),nullptr);v.setProperty("bpm",s.bpm,nullptr);}
void writeNotes(juce::ValueTree& v,const wee::Composition&c){v.setProperty("seed",juce::int64(c.settings.seed),nullptr);for(auto& n:c.notes){juce::ValueTree note("Note");note.setProperty("beat",n.beat,nullptr);note.setProperty("length",n.length,nullptr);note.setProperty("beatBits",juce::String::toHexString(juce::int64(std::bit_cast<uint64_t>(n.beat))),nullptr);note.setProperty("lengthBits",juce::String::toHexString(juce::int64(std::bit_cast<uint64_t>(n.length))),nullptr);note.setProperty("pitch",n.pitch,nullptr);note.setProperty("velocity",n.velocity,nullptr);v.appendChild(note,nullptr);}}
bool readNotes(const juce::ValueTree&v,wee::Composition&c){if(v.getNumChildren()>2048)return false;for(auto n:v){wee::Note note{double(n["beat"]),double(n["length"]),int(n["pitch"]),int(n["velocity"])};if(n.hasProperty("beatBits"))note.beat=std::bit_cast<double>(uint64_t(n["beatBits"].toString().getHexValue64()));if(n.hasProperty("lengthBits"))note.length=std::bit_cast<double>(uint64_t(n["lengthBits"].toString().getHexValue64()));if(!n.hasType("Note")||!std::isfinite(note.beat)||!std::isfinite(note.length)||note.beat<0||note.length<=0||note.beat+note.length>c.settings.bars*4+1.0e-8||note.pitch<0||note.pitch>127||note.velocity<1||note.velocity>127)return false;c.notes.push_back(note);}wee::sort(c);return true;}
}
void WeeProcessor::getStateInformation(juce::MemoryBlock&out){auto tree=params.copyState();tree.setProperty("version",3,nullptr);tree.setProperty("nextSeed",juce::int64(seed.load()),nullptr);auto a=snapshotArrangement();juce::ValueTree song("Arrangement");writeSettings(song,a.context);for(int i=0;i<8;++i){juce::ValueTree layer("Layer");layer.setProperty("index",i,nullptr);writeNotes(layer,a.layers[size_t(i)]);song.appendChild(layer,nullptr);}tree.appendChild(song,nullptr);auto xml=tree.createXml();copyXmlToBinary(*xml,out);}
void WeeProcessor::setStateInformation(const void*data,int size){
 if(!data||size<=0||size>4*1024*1024)return;auto xml=getXmlFromBinary(data,size);if(!xml)return;auto tree=juce::ValueTree::fromXml(*xml);if(!tree.hasType("WEE"))return;int version=int(tree["version"]);if(version!=1&&version!=2&&version!=3)return;
 // Build a full parameter tree first; missing Phase 2 controls get their defaults.
 juce::ValueTree restored("WEE");for(auto* raw:getParameters())if(auto* parameter=dynamic_cast<juce::RangedAudioParameter*>(raw)){
  double value=parameter->convertFrom0to1(parameter->getDefaultValue());auto input=tree.getChildWithProperty("id",parameter->paramID);if(input.isValid())value=double(input["value"]);
  auto range=parameter->getNormalisableRange();if(!std::isfinite(value)||value<range.start||value>range.end)return;juce::ValueTree v("PARAM");v.setProperty("id",parameter->paramID,nullptr);v.setProperty("value",range.snapToLegalValue(float(value)),nullptr);restored.appendChild(v,nullptr);
 }
 auto value=[&](const char* id){return double(restored.getChildWithProperty("id",id)["value"]);};wee::Settings settings;settings.key=int(value("key"));settings.scale=int(value("scale"));settings.style=int(value("style"));settings.bars=value("bars")>0?16:8;settings.bpm=value("bpm");
 wee::Arrangement a;
 if(version==1){auto song=tree.getChildWithName("Song");if(!song.isValid())return;settings.bars=int(song["bars"]);settings.seed=uint32_t(juce::int64(song["seed"]));settings.bpm=double(song["bpm"]);if((settings.bars!=8&&settings.bars!=16)||!std::isfinite(settings.bpm)||settings.bpm<20||settings.bpm>999)return;
  a=wee::generateArrangement(settings);a.context=settings;a.layers[0]={settings,{}};if(!readNotes(song,a.layers[0]))return;
 }else{auto song=tree.getChildWithName("Arrangement");if(!song.isValid()||song.getNumChildren()!=8)return;settings.bars=int(song["bars"]);settings.key=int(song["key"]);settings.scale=int(song["scale"]);settings.style=int(song["style"]);settings.seed=uint32_t(juce::int64(song["seed"]));settings.bpm=double(song["bpm"]);
  if((settings.bars!=8&&settings.bars!=16)||settings.key<0||settings.key>11||settings.scale<0||settings.scale>1||settings.style<0||settings.style>8||!std::isfinite(settings.bpm)||settings.bpm<20||settings.bpm>999)return;
  a.context=settings;std::array<bool,8> found{};for(auto layer:song){int i=int(layer["index"]);if(!layer.hasType("Layer")||i<0||i>=8||found[size_t(i)])return;found[size_t(i)]=true;a.layers[size_t(i)].settings=settings;a.layers[size_t(i)].settings.seed=uint32_t(juce::int64(layer["seed"]));if(!readNotes(layer,a.layers[size_t(i)]))return;}
 }
 params.replaceState(restored);seed=version>=2?uint32_t(juce::int64(tree["nextSeed"])):settings.seed+1;replaceArrangement(std::move(a));generationStatus=0;
}
juce::AudioProcessorEditor* WeeProcessor::createEditor(){return new WeeEditor(*this);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new WeeProcessor();}
