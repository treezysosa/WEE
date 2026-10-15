#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "Music/Composition.h"
#include "Instruments/Piano.h"
class WeeProcessor : public juce::AudioProcessor {
public:
 WeeProcessor();
 juce::AudioProcessorValueTreeState params;
 juce::Synthesiser synth;
 // Fixed storage, protected by a try-lock on the audio thread; edits never wait there.
 juce::SpinLock contentLock;
 wee::Composition content;
 std::atomic<bool> preview{false}, unsupportedMeter{false};
 std::atomic<double> tempo{120};
 void generate(); wee::Composition snapshot(); void replace(wee::Composition);
 void prepareToPlay(double,int) override; void releaseResources() override{}
 void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
 bool isBusesLayoutSupported(const BusesLayout& l) const override{return l.getMainOutputChannelSet()==juce::AudioChannelSet::stereo();}
 juce::AudioProcessorEditor* createEditor() override; bool hasEditor() const override{return true;}
 const juce::String getName() const override{return "WEE";}
 bool acceptsMidi() const override{return true;} bool producesMidi() const override{return false;} bool isMidiEffect() const override{return false;}
 double getTailLengthSeconds() const override{return 0.5;}
 int getNumPrograms() override{return 1;} int getCurrentProgram() override{return 0;} void setCurrentProgram(int) override{}
 const juce::String getProgramName(int) override{return "Default";} void changeProgramName(int,const juce::String&) override{}
 void getStateInformation(juce::MemoryBlock&) override; void setStateInformation(const void*,int) override;
private:
 double previewBeat=0; bool wasPlaying=false; uint32_t seed=1; juce::SmoothedValue<float> gain;
};
