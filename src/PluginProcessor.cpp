#include "PluginProcessor.h"
#include "PluginEditor.h"
WeeProcessor::WeeProcessor():AudioProcessor(BusesProperties().withOutput("Stereo",juce::AudioChannelSet::stereo(),true)),params(*this,nullptr,"WEE",{
 std::make_unique<juce::AudioParameterChoice>("style","Style",juce::StringArray{"Pop","Dark Pop","Indie Pop"},1),
 std::make_unique<juce::AudioParameterInt>("key","Key",0,11,9),
 std::make_unique<juce::AudioParameterChoice>("scale","Scale",juce::StringArray{"Minor","Major"},0),
 std::make_unique<juce::AudioParameterChoice>("bars","Bars",juce::StringArray{"8","16"},0),
 std::make_unique<juce::AudioParameterFloat>("bpm","Preview BPM",40.f,240.f,120.f),
 std::make_unique<juce::AudioParameterBool>("sync","Host sync",true),
 std::make_unique<juce::AudioParameterFloat>("gain","Volume",0.f,1.f,0.8f)}) {
 for(int i=0;i<24;++i)synth.addVoice(new PianoVoice());synth.addSound(new PianoSound()); synth.setMinimumRenderingSubdivisionSize(1, true); generate();
}
void WeeProcessor::generate(){wee::Settings s;s.seed=seed++;s.key=int(*params.getRawParameterValue("key"));s.scale=int(*params.getRawParameterValue("scale"));s.style=int(*params.getRawParameterValue("style"));s.bars=*params.getRawParameterValue("bars")>0?16:8;s.bpm=*params.getRawParameterValue("bpm");replace(wee::generate(s));}
wee::Composition WeeProcessor::snapshot(){const juce::SpinLock::ScopedLockType l(contentLock);return content;}
void WeeProcessor::replace(wee::Composition c){const juce::SpinLock::ScopedLockType l(contentLock);content=std::move(c);}
void WeeProcessor::prepareToPlay(double sr,int){synth.setCurrentPlaybackSampleRate(sr);gain.reset(sr,0.02);gain.setCurrentAndTargetValue(*params.getRawParameterValue("gain"));previewBeat=0;wasPlaying=false;}
void WeeProcessor::processBlock(juce::AudioBuffer<float>& audio,juce::MidiBuffer& midi){
 juce::ScopedNoDenormals dn;audio.clear();if(getSampleRate()<=0 || audio.getNumSamples()==0){midi.clear();return;}bool host=false;double beat=previewBeat,bpm=*params.getRawParameterValue("bpm");bool bad=false;
 if(*params.getRawParameterValue("sync")>0.5f)if(auto* ph=getPlayHead())if(auto pos=ph->getPosition()){
  host=pos->getIsPlaying();if(auto t=pos->getBpm())if(std::isfinite(*t)&&*t>0)bpm=juce::jlimit(20.0,999.0,*t);
  if(host)if(auto p=pos->getPpqPosition())if(std::isfinite(*p))beat=*p;
  if(auto ts=pos->getTimeSignature())bad=ts->numerator!=4||ts->denominator!=4;
 }
 tempo=bpm;unsupportedMeter=bad;bool playing=(host||preview.load())&&!bad;
 if(bad){synth.allNotesOff(0,true);}
 if(wasPlaying&&!playing){synth.allNotesOff(0,true);previewBeat=0;}
 if(playing){juce::SpinLock::ScopedTryLockType lock(contentLock);if(lock.isLocked()){
  double step=bpm/(60*getSampleRate()),end=beat+audio.getNumSamples()*step,loop=content.settings.bars*4;
  for(auto& n:content.notes)for(int off=0;off<2;++off){double event=n.beat+(off?n.length:0);double at=event+std::ceil((beat-event)/loop)*loop;
   while(at<end){int sample=juce::jlimit(0,audio.getNumSamples()-1,int(std::lround((at-beat)/step)));midi.addEvent(off?juce::MidiMessage::noteOff(1,n.pitch):juce::MidiMessage::noteOn(1,n.pitch,(juce::uint8)n.velocity),sample);at+=loop;}
  }
  previewBeat=std::fmod(end,loop);
 }}wasPlaying=playing;synth.renderNextBlock(audio,midi,0,audio.getNumSamples());midi.clear();gain.setTargetValue(*params.getRawParameterValue("gain"));for(int i=0;i<audio.getNumSamples();++i){auto g=gain.getNextValue();for(int c=0;c<audio.getNumChannels();++c)audio.getWritePointer(c)[i]*=g;}
}
void WeeProcessor::getStateInformation(juce::MemoryBlock& out){auto tree=params.copyState();tree.setProperty("version",1,nullptr);auto c=snapshot();juce::ValueTree song("Song");song.setProperty("bars",c.settings.bars,nullptr);song.setProperty("seed",juce::int64(c.settings.seed),nullptr);song.setProperty("bpm",c.settings.bpm,nullptr);for(auto&n:c.notes){juce::ValueTree v("Note");v.setProperty("beat",n.beat,nullptr);v.setProperty("length",n.length,nullptr);v.setProperty("pitch",n.pitch,nullptr);v.setProperty("velocity",n.velocity,nullptr);song.appendChild(v,nullptr);}tree.addChild(song,-1,nullptr);auto xml=tree.createXml();copyXmlToBinary(*xml,out);}
void WeeProcessor::setStateInformation(const void* data,int size){auto xml=getXmlFromBinary(data,size);if(!xml)return;auto tree=juce::ValueTree::fromXml(*xml);if(!tree.hasType("WEE")||int(tree["version"])!=1)return;auto song=tree.getChildWithName("Song");if(!song.isValid())return;wee::Composition c;c.settings.bars=int(song["bars"]);if(c.settings.bars!=8&&c.settings.bars!=16)return;c.settings.seed=uint32_t(juce::int64(song["seed"]));c.settings.bpm=juce::jlimit(40.0,240.0,double(song["bpm"]));for(auto v:song){wee::Note n{double(v["beat"]),double(v["length"]),int(v["pitch"]),int(v["velocity"])};if(!std::isfinite(n.beat)||!std::isfinite(n.length)||n.beat<0||n.length<=0||n.beat+n.length>c.settings.bars*4||n.pitch<0||n.pitch>127||n.velocity<1||n.velocity>127||c.notes.size()>4096)return;c.notes.push_back(n);}tree.removeChild(song,nullptr);params.replaceState(tree);seed=c.settings.seed+1;replace(std::move(c));}
juce::AudioProcessorEditor* WeeProcessor::createEditor(){return new WeeEditor(*this);}juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new WeeProcessor();}
