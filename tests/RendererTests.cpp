#include "../src/Export/Renderer.h"
#include <iostream>
#include <iomanip>
#include <stdexcept>
namespace {
void check(bool b,const char*m){if(!b)throw std::runtime_error(m);}
double energy(const wee::StereoAudio&a){double n=0;for(auto&c:a.channels)for(float x:c){check(std::isfinite(x),"finite render");n+=double(x)*x;}return n;}
void set(WeeProcessor&p,const juce::String&id,float v){auto* param=p.params.getParameter(id);param->setValueNotifyingHost(param->convertTo0to1(v));}
}
void runRenderTests(){
 WeeProcessor p;p.setRateAndBufferSizeDetails(8000,256);p.prepareToPlay(8000,256);set(p,"sync",0);set(p,"bpm",240);set(p,"fx_enable",1);set(p,"fx_reverb",.6f);set(p,"fx_delay",.5f);set(p,"wav_tail",1);set(p,"wav_tailseconds",1);
 auto request=captureRender(p);request.audition=true;request.flipSettings.half=true;request.flipSettings.reverse=true;request.flipSettings.transpose=5;
 auto data=renderLoop(request,request.mask);size_t expected=size_t(std::llround(request.bars*4*60/request.bpm*request.rate));
 check(data.original.size()==expected&&data.flipped.size()==expected,"exact original and FLIP loop frame counts");check(data.originalTail.size()==8000&&data.flippedTail.size()==8000,"tails are separate lengths");check(energy(data.original)>.01&&energy(data.flipped)>.01,"real instrument and flip rendering");check(energy(data.originalTail)>1.0e-10,"instrument/effect release tail rendered");
 double delta=0;for(size_t i=0;i<expected;++i)delta+=std::abs(data.original.channels[0][i]-data.flipped.channels[0][i]);check(delta>.01,"FLIP genuinely transforms samples");check(data.original.channels[0].front()==0&&data.original.channels[0].back()==0,"seamless endpoints");
 auto directory=juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory().getChildFile("wav-roundtrip-tests");check(directory.createDirectory().wasOk(),"test WAV directory");
 for(int bits=0;bits<3;++bits){auto file=directory.getChildFile("render-"+juce::String(bits)+".wav");
#if JUCE_WINDOWS
 std::filesystem::path path(file.getFullPathName().toWideCharPointer());
#else
 std::filesystem::path path(file.getFullPathName().toStdString());
#endif
 wee::writeWav(path,data.original,8000,wee::WavDepth(bits));juce::WavAudioFormat format;std::unique_ptr<juce::AudioFormatReader>reader(format.createReaderFor(new juce::FileInputStream(file),true));check(reader&&reader->lengthInSamples==juce::int64(expected)&&reader->numChannels==2&&reader->sampleRate==8000,"JUCE independent WAV import");check(reader->bitsPerSample==unsigned(bits==0?16:bits==1?24:32),"WAV precision");check(reader->usesFloatingPointData==(bits==2),"32-bit must be float");}
 // V3 parameters and lossless notes round-trip, V2 states default effects to bypass.
 juce::MemoryBlock state;p.getStateInformation(state);WeeProcessor recalled;recalled.setStateInformation(state.getData(),int(state.getSize()));auto expectedReverb=p.params.getRawParameterValue("fx_reverb")->load(),actualReverb=recalled.params.getRawParameterValue("fx_reverb")->load();if(expectedReverb!=actualReverb)std::cerr<<"Reverb expected "<<std::setprecision(12)<<expectedReverb<<" recalled "<<actualReverb<<"\n";check(actualReverb==expectedReverb,"effect state recall");check(recalled.snapshot().notes==p.snapshot().notes,"V3 exact notes");
 auto xml=juce::AudioProcessor::getXmlFromBinary(state.getData(),int(state.getSize()));auto legacy=juce::ValueTree::fromXml(*xml);legacy.setProperty("version",2,nullptr);for(auto spec:wee::effectParameters){auto child=legacy.getChildWithProperty("id",spec.id);legacy.removeChild(child,nullptr);}auto legacyXml=legacy.createXml();juce::MemoryBlock old;juce::AudioProcessor::copyXmlToBinary(*legacyXml,old);recalled.setStateInformation(old.getData(),int(old.getSize()));check(*recalled.params.getRawParameterValue("fx_enable")==0&&recalled.snapshot().notes==p.snapshot().notes,"Phase 2 migration preserves notes and bypasses new FX");
 // Frozen A/B publication is invalidated by edits, while master gain remains live.
 auto pair=std::make_unique<WeeAudition>();pair->original=std::move(data.original);pair->flipped=std::move(data.flipped);pair->bpm=p.exportTempo();pair->bars=8;pair->rate=8000;pair->token=p.changeToken.load();p.publishAudition(std::move(pair));check(p.auditionReady(),"audition published");p.auditionMode=2;p.preview=true;juce::AudioBuffer<float> block(2,256);juce::MidiBuffer midi;p.processBlock(block,midi);double sampleEnergy=0;for(int i=0;i<256;++i)sampleEnergy+=std::abs(block.getSample(0,i));check(sampleEnergy>0,"cached FLIP audition plays");p.preview=false;p.processBlock(block,midi);p.processBlock(block,midi);double stopped=0;for(int i=0;i<256;++i)stopped+=std::abs(block.getSample(0,i));check(stopped==0,"cached preview stop fades to silence");set(p,"gain",.5f);check(p.auditionReady(),"master volume can change cached audition");set(p,"fx_wow",.2f);check(!p.auditionReady()&&p.auditionMode==0,"edits invalidate stale audition");
 std::atomic<bool> cancel{true};bool cancelled=false;try{renderLoop(request,request.mask,&cancel);}catch(...){cancelled=true;}check(cancelled,"renderer cancellation");
 // Fractional frame rounding, 8/16 bars, and a forced layer export regardless of mute.
 set(p,"bars",1);p.generate();request=captureRender(p);request.audition=false;request.flip=false;request.separateTail=false;request.bpm=137;request.bars=16;auto mask=request.mask;mask.fill(false);mask[3]=true;auto stem=renderLoop(request,mask);check(stem.original.size()==size_t(std::llround(16*4*60.0/137*8000)),"fractional BPM nominal frame rounding");check(energy(stem.original)>0,"individual stem instrument audible");

 auto atHostRate=request;atHostRate.rate=48000;atHostRate.bpm=240;auto hostRateAudio=renderLoop(atHostRate,mask);check(hostRateAudio.original.size()==size_t(std::llround(16*4*60.0/240*48000)),"48 kHz host-rate loop duration");check(energy(hostRateAudio.original)>0,"48 kHz actual DSP render");
 // Background stem export writes actual files and leaves the live arrangement untouched.
 auto snapshot=p.snapshotArrangement();auto jobRequest=captureRender(p);jobRequest.bpm=240;jobRequest.rate=8000;jobRequest.stems=true;jobRequest.separateTail=true;jobRequest.tailSeconds=1;jobRequest.directory=directory;jobRequest.mask.fill(false);jobRequest.mask[0]=jobRequest.mask[3]=true;
 {WeeRenderJob job(p);job.start(jobRequest);auto deadline=juce::Time::getMillisecondCounterHiRes()+20000;while(job.busy()&&juce::Time::getMillisecondCounterHiRes()<deadline)juce::Thread::sleep(5);check(!job.busy(),"background rendering completion");check(job.files().size()==4,"two stems plus separate tails");check(job.message().startsWith("WAV export complete"),"worker reported real success");job.start(jobRequest);job.cancel();deadline=juce::Time::getMillisecondCounterHiRes()+20000;while(job.busy()&&juce::Time::getMillisecondCounterHiRes()<deadline)juce::Thread::sleep(5);check(!job.busy(),"worker cancellation finishes");check(p.snapshot().notes==snapshot.layers[0].notes,"background jobs preserve live content");}

 // Boundary note-offs must precede note-ons at the same sample/pitch.
 WeeProcessor boundary;wee::Composition melody;melody.settings.bars=8;melody.notes={{0,1,60,100},{31,1,60,100}};boundary.replace(melody);boundary.setRateAndBufferSizeDetails(8000,256);boundary.prepareToPlay(8000,256);
 struct BoundaryClock:juce::AudioPlayHead{double beat=32;juce::Optional<PositionInfo>getPosition()const override{PositionInfo p;p.setIsPlaying(true);p.setBpm(120.0);p.setPpqPosition(beat);return p;}}clock;boundary.setPlayHead(&clock);midi.clear();boundary.processBlock(block,midi);double held=0;for(int k=0;k<8;++k){clock.beat+=256.0*120/(60*8000);midi.clear();boundary.processBlock(block,midi);held=0;for(int i=0;i<256;++i)held+=std::abs(block.getSample(0,i));}check(held>.01,"same-pitch loop retrigger survives boundary note-off");boundary.setPlayHead(nullptr);
 auto beforeMeter=p.snapshot().notes;p.unsupportedMeter=true;check(!p.generate()&&!p.regenerateLayer(0),"unsupported meter blocks generation");bool rejectedMeter=false;try{captureRender(p);}catch(...){rejectedMeter=true;}check(rejectedMeter&&p.snapshot().notes==beforeMeter,"unsupported meter blocks export without changing music");p.unsupportedMeter=false;
 std::cout<<"PASS: actual-engine WAV rendering, exact bars/tails, real FLIP, JUCE WAV parser, V2/V3 state, A/B publication, stems and cancellation\n";
}
