#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "Music/Composition.h"
#include "Instruments/Piano.h"
#include "Effects/Parameters.h"
#include "Effects/Flip.h"
struct WeeAudition {wee::StereoAudio original,flipped;double rate=48000,bpm=120;int bars=8;uint64_t token=0;};
class WeeProcessor : public juce::AudioProcessor,private juce::AudioProcessorValueTreeState::Listener {
public:
 WeeProcessor(); ~WeeProcessor() override;
 std::atomic<int> auditionMode{0}; // 0 live, 1 cached original, 2 cached FLIP
 std::atomic<uint64_t> changeToken{1},clipToken{0};
 double exportTempo() const; double exportRate() const{return preparedRate.load();}
 bool auditionReady() const{return clipToken.load()==changeToken.load();}
 void publishAudition(std::unique_ptr<WeeAudition>);
 wee::EffectSettings effectSettings() const; wee::FlipSettings flipSettings() const;
 juce::AudioProcessorValueTreeState params;
 std::atomic<bool> preview{false}, unsupportedMeter{false};
 std::atomic<double> tempo{120};
 // 0: ready; 1: unlock layers before changing harmonic context; 2: selected layer locked.
 std::atomic<int> generationStatus{0};
 static juce::String layerId(int,const juce::String&);
 bool generate(); bool regenerateLayer(int);
 wee::Composition snapshot(); void replace(wee::Composition);
 wee::Arrangement snapshotArrangement(); void replaceArrangement(wee::Arrangement);
 std::array<bool,8> audibleLayers() const;
 void prepareToPlay(double,int) override; void releaseResources() override{}
 void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
 bool isBusesLayoutSupported(const BusesLayout& l) const override{return l.getMainInputChannelSet().isDisabled()&&l.getMainOutputChannelSet()==juce::AudioChannelSet::stereo();}
 juce::AudioProcessorEditor* createEditor() override; bool hasEditor() const override{return true;}
 const juce::String getName() const override{return "WEE";}
 bool acceptsMidi() const override{return true;} bool producesMidi() const override{return false;} bool isMidiEffect() const override{return false;}
 double getTailLengthSeconds() const override;
 int getNumPrograms() override{return 1;} int getCurrentProgram() override{return 0;} void setCurrentProgram(int) override{}
 const juce::String getProgramName(int) override{return "Default";} void changeProgramName(int,const juce::String&) override{}
 void getStateInformation(juce::MemoryBlock&) override; void setStateInformation(const void*,int) override;
private:
 static juce::AudioProcessorValueTreeState::ParameterLayout layout();
 struct LayerParameters {std::atomic<float>* enabled=nullptr,*mute=nullptr,*solo=nullptr,*volume=nullptr,*pan=nullptr,*instrument=nullptr,*locked=nullptr;};
 std::array<LayerParameters,8> controls;
 std::array<juce::Synthesiser,8> engines;
 std::array<juce::AudioBuffer<float>,8> audio;
 std::array<juce::MidiBuffer,8> events;
 std::array<juce::SmoothedValue<float>,8> left,right;
 std::array<std::atomic<uint32_t>,8> revision{};
 std::array<uint32_t,8> renderedRevision{};
 std::array<int,8> previousInstrument{};
 std::array<bool,8> previousAudible{};
 juce::SpinLock contentLock; wee::Arrangement content;
 double previewBeat=0,expectedHostBeat=0; bool wasPlaying=false,wasHost=false;
 std::atomic<uint32_t> seed{1};juce::SmoothedValue<float> gain;
 wee::EffectChain effects;
 std::array<std::atomic<float>*,10> effectRefs{};
 std::atomic<float>* bpmParam=nullptr,*syncParam=nullptr,*gainParam=nullptr;
 std::atomic<double> preparedRate{48000};std::atomic<bool> hostTempoAvailable{false};
 juce::SpinLock auditionLock;std::unique_ptr<WeeAudition> audition;
 juce::SmoothedValue<float> auditionBlend,auditionGate;double auditionBeat=0;
 void parameterChanged(const juce::String&,float) override;
 int preparedBlock=512,playbackBars=8;
};
