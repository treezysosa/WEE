#include "Renderer.h"
#include <filesystem>
namespace {
void set(WeeProcessor&p,const juce::String&id,float value){if(auto* parameter=p.params.getParameter(id))parameter->setValueNotifyingHost(parameter->convertTo0to1(value));}
void check(const std::atomic<bool>* flag){if(wee::cancelled(flag))throw std::runtime_error("Cancelled");}
class RenderClock : public juce::AudioPlayHead {
public:double bpm=120,beat=0;bool playing=true;
 juce::Optional<PositionInfo> getPosition()const override{PositionInfo p;p.setBpm(bpm);p.setPpqPosition(beat);p.setIsPlaying(playing);p.setTimeSignature(TimeSignature{4,4});return p;}
};
std::filesystem::path native(const juce::File& file){
#if JUCE_WINDOWS
 return std::filesystem::path(file.getFullPathName().toWideCharPointer());
#else
 return std::filesystem::path(file.getFullPathName().toStdString());
#endif
}
float peak(const wee::StereoAudio&a){float p=0;for(auto& c:a.channels)for(float value:c){if(!std::isfinite(value))throw std::runtime_error("Non-finite rendered sample");p=std::max(p,std::abs(value));}return p;}
}
WeeRenderRequest captureRender(WeeProcessor&p){
 if(p.unsupportedMeter.load())throw std::runtime_error("WEE supports 4/4 only. Change the host time signature before rendering.");
 WeeRenderRequest q;q.token=p.changeToken.load();p.getStateInformation(q.state);q.rate=p.exportRate();
 auto xml=juce::AudioProcessor::getXmlFromBinary(q.state.getData(),int(q.state.getSize()));if(!xml)throw std::runtime_error("Cannot capture render state");auto tree=juce::ValueTree::fromXml(*xml);
 auto get=[&](const char*id){return float(tree.getChildWithProperty("id",id)["value"]);};
 q.bpm=get("sync")>0.5f?p.exportTempo():double(get("bpm"));q.bars=int(tree.getChildWithName("Arrangement")["bars"]);q.master=get("gain");
 bool solo=false;for(int i=0;i<8;++i)solo|=get(WeeProcessor::layerId(i,"enabled").toRawUTF8())>0.5f&&get(WeeProcessor::layerId(i,"solo").toRawUTF8())>0.5f;
 for(int i=0;i<8;++i)q.mask[size_t(i)]=get(WeeProcessor::layerId(i,"enabled").toRawUTF8())>0.5f&&get(WeeProcessor::layerId(i,"mute").toRawUTF8())<0.5f&&(!solo||get(WeeProcessor::layerId(i,"solo").toRawUTF8())>0.5f);
 q.flipSettings.half=get("flip_half")>0.5f;q.flipSettings.reverse=get("flip_reverse")>0.5f;q.flipSettings.transpose=int(get("flip_pitch"));q.flipSettings.tape=get("flip_tape");q.flipSettings.wow=get("flip_wow");q.flipSettings.lowpass=get("flip_lowpass");q.flipSettings.reverb=get("flip_reverb");q.flipSettings.vintage=get("flip_vintage");
 q.flip=get("flip_export")>0.5f;q.seamless=get("wav_seamless")>0.5f;q.separateTail=get("wav_tail")>0.5f;q.tailSeconds=get("wav_tailseconds");q.protect=get("wav_protect")>0.5f;q.depth=wee::WavDepth(int(get("wav_depth")));return q;
}
WeeRendered renderLoop(const WeeRenderRequest&q,const std::array<bool,8>&mask,const std::atomic<bool>*cancel,std::function<void(double)>progress){
 if(!std::isfinite(q.rate)||q.rate<8000||q.rate>384000||!std::isfinite(q.bpm)||q.bpm<20||q.bpm>999||(q.bars!=8&&q.bars!=16))throw std::runtime_error("Unsupported render rate, tempo or bar count");
 check(cancel);const int64_t frames=int64_t(std::llround(q.bars*4*60/q.bpm*q.rate));
 // Explicit bounds prevent multi-gigabyte allocations for extreme host settings.
 if(frames<1||frames>16000000)throw std::runtime_error("Render exceeds memory limit. Reduce bars/sample rate or raise tempo.");
 size_t n=size_t(frames),tail=q.separateTail?size_t(std::llround(std::clamp(q.tailSeconds,1.0,30.0)*q.rate)):0;
 WeeRendered result;result.original.resize(n);if(tail)result.originalTail.resize(tail);
 WeeProcessor instrument;instrument.setStateInformation(q.state.getData(),int(q.state.getSize()));set(instrument,"gain",1);set(instrument,"sync",1);
 for(int i=0;i<8;++i){set(instrument,WeeProcessor::layerId(i,"enabled"),mask[size_t(i)]?1.f:0.f);set(instrument,WeeProcessor::layerId(i,"mute"),0);set(instrument,WeeProcessor::layerId(i,"solo"),0);}
 RenderClock clock;clock.bpm=q.rate*q.bars*4*60/double(frames);instrument.setPlayHead(&clock);instrument.setRateAndBufferSizeDetails(q.rate,512);instrument.prepareToPlay(q.rate,512);
 juce::AudioBuffer<float> block(2,512);juce::MidiBuffer events;events.ensureSize(65536);
 // Warm the actual instruments + live effect chain for two cycles before capture.
 int64_t total=frames*3+int64_t(tail);
 for(int64_t start=0;start<total;start+=512){check(cancel);int count=int(std::min<int64_t>(512,total-start));block.setSize(2,count,false,false,true);clock.beat=double(start)*q.bars*4/double(frames);clock.playing=start<frames*3;
  // Split at the loop/tail boundary so no extra notes enter the tail.
  if(start<frames*3&&start+count>frames*3)count=int(frames*3-start);
  block.setSize(2,count,false,false,true);events.clear();instrument.processBlock(block,events);
  for(int i=0;i<count;++i){int64_t at=start+i;if(at>=frames*2&&at<frames*3)for(int c=0;c<2;++c)result.original.channels[size_t(c)][size_t(at-frames*2)]=block.getSample(c,i);else if(at>=frames*3)for(int c=0;c<2;++c)result.originalTail.channels[size_t(c)][size_t(at-frames*3)]=block.getSample(c,i);}
  if(progress)progress(.5*double(start+count)/double(total));
  // The increment uses actual count when a boundary split shortened this block.
  start+=count-512;
 }
 instrument.setPlayHead(nullptr);
 if(q.flip||q.audition){result.flipped=wee::transform(result.original,q.rate,q.flipSettings,cancel);if(tail)result.flippedTail.resize(tail);
  auto settings=wee::flipEffects(q.flipSettings);wee::EffectChain flip;flip.prepare(q.rate);flip.set(settings);
  std::array<std::array<float,512>,2> scratch{};int64_t flipTotal=frames*3+int64_t(tail);
  for(int64_t start=0;start<flipTotal;start+=512){check(cancel);int count=int(std::min<int64_t>(512,flipTotal-start));if(start<frames*3&&start+count>frames*3)count=int(frames*3-start);
   for(int i=0;i<count;++i)for(int c=0;c<2;++c)scratch[size_t(c)][size_t(i)]=start+i<frames*3?result.flipped.channels[size_t(c)][size_t((start+i)%frames)]:0;
   flip.process(scratch[0].data(),scratch[1].data(),count,q.bpm);
   // Store capture separately: overwriting the source while wrapping would alter later frames.
   for(int i=0;i<count;++i){int64_t at=start+i;for(int c=0;c<2;++c){if(at>=frames*2&&at<frames*3)result.flipped.channels[size_t(c)][size_t(at-frames*2)]=scratch[size_t(c)][size_t(i)];else if(at>=frames*3)result.flippedTail.channels[size_t(c)][size_t(at-frames*3)]=scratch[size_t(c)][size_t(i)];}}
   if(progress)progress(.5+.5*double(start+count)/double(flipTotal));start+=count-512;
  }
 }
 if(q.seamless||q.audition){wee::softenBoundary(result.original,q.rate);if(q.flip||q.audition)wee::softenBoundary(result.flipped,q.rate);if(tail){wee::softenBoundary(result.originalTail,q.rate);if(q.flip||q.audition)wee::softenBoundary(result.flippedTail,q.rate);}}if(progress)progress(1);return result;
}
void WeeRenderJob::setMessage(juce::String text){const std::lock_guard<std::mutex>lock(resultLock);status=std::move(text);}
juce::String WeeRenderJob::message()const{const std::lock_guard<std::mutex>lock(resultLock);return status;}
juce::File WeeRenderJob::folder()const{const std::lock_guard<std::mutex>lock(resultLock);return resultFolder;}
juce::StringArray WeeRenderJob::files()const{const std::lock_guard<std::mutex>lock(resultLock);return resultFiles;}
void WeeRenderJob::start(WeeRenderRequest q){if(running)return;if(worker.joinable())worker.join();cancelFlag=false;completed=0;running=true;setMessage("Rendering a snapshot in the background...");try{worker=std::thread([this,q=std::move(q)]()mutable{run(std::move(q));});}catch(const std::exception&e){running=false;setMessage("Could not start render worker: "+juce::String(e.what()));}}
void WeeRenderJob::run(WeeRenderRequest q){
 juce::File partial;
 try{
  if(q.audition){auto data=renderLoop(q,q.mask,&cancelFlag,[&](double x){completed=x;});check(&cancelFlag);auto pair=std::make_unique<WeeAudition>();pair->original=std::move(data.original);pair->flipped=std::move(data.flipped);pair->rate=q.rate;pair->bpm=q.bpm;pair->bars=q.bars;pair->token=q.token;owner.publishAudition(std::move(pair));setMessage(owner.auditionReady()?"A/B ready. ORIGINAL / FLIPPED starts preview; LIVE returns to synthesis.":"Settings changed during render. Render A/B again for the current snapshot.");}
  else{
   auto directory=q.directory.getNonexistentChildFile("WEE-"+juce::Time::getCurrentTime().formatted("%Y%m%d-%H%M%S"),"",false);if(directory.createDirectory().failed())throw std::runtime_error("Cannot create export folder");
   {const std::lock_guard<std::mutex>lock(resultLock);resultFolder=directory;resultFiles.clear();}
   std::vector<int> layers;if(q.stems){for(int i=0;i<8;++i)if(q.mask[size_t(i)])layers.push_back(i);}else layers.push_back(q.layer);if(layers.empty())throw std::runtime_error("No enabled audible layers to export");
   bool attenuated=false;
   for(size_t job=0;job<layers.size();++job){check(&cancelFlag);int layer=layers[job];auto mask=q.mask;if(layer>=0){mask.fill(false);mask[size_t(layer)]=true;}
    auto data=renderLoop(q,mask,&cancelFlag,[&](double x){completed=(double(job)+x*.92)/double(layers.size());});auto& audio=q.flip?data.flipped:data.original;auto& tail=q.flip?data.flippedTail:data.originalTail;
    float level=q.master,max=std::max(peak(audio),peak(tail))*q.master;if(q.protect&&max>.98f){level*=.98f/max;attenuated=true;}
    juce::String name=layer<0?"WEE-Mix":juce::String("WEE-")+wee::layerNames[size_t(layer)];if(q.flip)name+="-FLIP";
    auto write=[&](const wee::StereoAudio& samples,const juce::String& suffix){auto target=directory.getChildFile(name+suffix+".wav");partial=target.withFileExtension("wav.partial");wee::writeWav(native(partial),samples,q.rate,q.depth,level,&cancelFlag);check(&cancelFlag);if(target.exists()||!partial.moveFileTo(target))throw std::runtime_error("Cannot finalize WAV file");partial=juce::File{};{const std::lock_guard<std::mutex>lock(resultLock);resultFiles.add(target.getFullPathName());}};
    write(audio,"");if(q.separateTail)write(tail,"-tail");completed=double(job+1)/double(layers.size());
   }
   auto info=directory.getChildFile("WEE-Render-Info.txt");juce::String text="WEE WAV export\nBars: "+juce::String(q.bars)+"\nBPM: "+juce::String(q.bpm,6)+"\nSample rate: "+juce::String(int(q.rate))+" Hz\nFormat: "+juce::String(q.depth==wee::WavDepth::pcm16?"16-bit PCM":q.depth==wee::WavDepth::pcm24?"24-bit PCM":"32-bit IEEE float")+"\nSeamless: "+juce::String(q.seamless?"yes":"no")+"\nWEE FLIP: "+juce::String(q.flip?"yes":"no")+"\nTail files: "+juce::String(q.separateTail?"yes":"no")+"\n";if(!info.replaceWithText(text))throw std::runtime_error("Could not write export information");
   setMessage("WAV export complete. "+juce::String(int(files().size()))+" files at "+juce::String(int(q.rate))+" Hz."+(attenuated?" Peak protection reduced the gain.":""));
  }
 }catch(const std::exception&e){if(partial.existsAsFile())partial.deleteFile();setMessage(juce::String(e.what())+(files().isEmpty()?"":". Completed files were retained."));}
 completed=1;running=false;
}
