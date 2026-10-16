#pragma once
#include "../PluginProcessor.h"
#include "Wav.h"
#include <functional>
#include <mutex>
#include <thread>
struct WeeRenderRequest {
 juce::MemoryBlock state;double rate=48000,bpm=120;int bars=8,layer=-1;
 bool stems=false,audition=false,flip=false,seamless=true,separateTail=false,protect=true;
 double tailSeconds=8;float master=0.8f;uint64_t token=0;
 wee::WavDepth depth=wee::WavDepth::pcm24;wee::FlipSettings flipSettings;
 std::array<bool,8> mask{};juce::File directory;
};
struct WeeRendered {wee::StereoAudio original,flipped,originalTail,flippedTail;};
WeeRenderRequest captureRender(WeeProcessor&);
WeeRendered renderLoop(const WeeRenderRequest&,const std::array<bool,8>&,const std::atomic<bool>* cancel=nullptr,std::function<void(double)> progress={});
class WeeRenderJob {
public:
 explicit WeeRenderJob(WeeProcessor& processor):owner(processor){}
 ~WeeRenderJob(){cancel();if(worker.joinable())worker.join();}
 void start(WeeRenderRequest);
 void cancel(){cancelFlag=true;}
 bool busy()const{return running.load();}
 double progress()const{return completed.load();}
 juce::String message()const;juce::File folder()const;juce::StringArray files()const;
private:
 WeeProcessor&owner;std::thread worker;
 std::atomic<bool>cancelFlag{false},running{false};std::atomic<double>completed{0};
 mutable std::mutex resultLock;juce::String status="Ready. Render A/B to compare the current mix and WEE FLIP.";
 juce::File resultFolder;juce::StringArray resultFiles;
 void setMessage(juce::String);
 void run(WeeRenderRequest);
};
