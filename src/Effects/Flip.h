#pragma once
#include "EffectChain.h"
#include <atomic>
#include <stdexcept>
namespace wee {
struct StereoAudio {std::array<std::vector<float>,2> channels;size_t size()const{return channels[0].size();}void resize(size_t n){for(auto& c:channels)c.resize(n);} };
struct FlipSettings {bool half=true,reverse=false;int transpose=0;float tape=.3f,wow=.15f,lowpass=9000,reverb=.2f,vintage=.2f;};
inline bool cancelled(const std::atomic<bool>* flag){return flag&&flag->load();}
inline StereoAudio transform(const StereoAudio& input,double rate,const FlipSettings& p,const std::atomic<bool>* cancel=nullptr){
 StereoAudio result;size_t n=input.size();result.resize(n);if(n==0)return result;
 double speed=p.half?.5:1,ratio=speed*std::pow(2.0,std::clamp(p.transpose,-24,24)/12.0);double grain=std::max(32.0,rate*.06);
 auto sample=[&](int ch,double position){position=std::fmod(position,double(n));if(position<0)position+=double(n);size_t a=size_t(position),b=(a+1)%n;float mix=float(position-double(a));return input.channels[size_t(ch)][a]*(1-mix)+input.channels[size_t(ch)][b]*mix;};
 for(size_t i=0;i<n;++i){if((i&4095u)==0&&cancelled(cancel))throw std::runtime_error("Cancelled");double phase=std::fmod(double(i)/grain,1.0),phase2=std::fmod(phase+.5,1.0);double w=.5-.5*std::cos(2*pi*phase);size_t destination=p.reverse?n-1-i:i;
  for(int ch=0;ch<2;++ch){float value;if(p.transpose==0)value=sample(ch,speed*double(i));else value=float(w*sample(ch,speed*double(i)+(ratio-speed)*phase*grain)+(1-w)*sample(ch,speed*double(i)+(ratio-speed)*phase2*grain));result.channels[size_t(ch)][destination]=value;}
 }
 return result;
}
inline EffectSettings flipEffects(const FlipSettings&p){EffectSettings fx;fx.enabled=p.tape>0||p.wow>0||p.lowpass<19999||p.reverb>0||p.vintage>0;fx.saturation=std::clamp(p.tape+p.vintage*.2f,0.f,1.f);fx.wow=p.wow;fx.lowpass=std::min(p.lowpass,20000-p.vintage*12000);fx.reverb=p.reverb;fx.vinyl=p.vintage*.35f;return fx;}
inline void softenBoundary(StereoAudio& audio,double rate){size_t n=audio.size();size_t fade=std::min(n/2,std::max(size_t(2),size_t(rate*.002)));for(size_t i=0;i<fade;++i){float g=float(.5-.5*std::cos(pi*double(i)/double(fade-1)));for(auto& c:audio.channels){c[i]*=g;c[n-1-i]*=g;}}}
}
