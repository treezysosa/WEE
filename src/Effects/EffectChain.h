#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>
namespace wee {
constexpr double pi=3.14159265358979323846;
struct EffectSettings {
 bool enabled=false;
 float reverb=0,delay=0,chorus=0,saturation=0,lowpass=20000,highpass=20,width=1,wow=0,vinyl=0;
};
// Allocation-free processing after prepare(). All public controls ramp over 20 ms.
class EffectChain {
 struct Ramp {float value=0,target=0,step=0;int remaining=0;void set(float t,int n){if(t!=target){target=t;remaining=n;step=(t-value)/float(n);}}float next(){if(remaining>0){value+=step;if(--remaining==0)value=target;}return value;}};
 struct Ring {
  std::vector<float> samples;size_t head=0;
  void prepare(size_t n){samples.assign(std::max(size_t(4),n),0);head=0;}
  float read(double delay)const{double p=double(head)-std::clamp(delay,1.0,double(samples.size()-2));while(p<0)p+=double(samples.size());size_t a=size_t(p),b=(a+1)%samples.size();float f=float(p-double(a));return samples[a]*(1-f)+samples[b]*f;}
  void push(float v){samples[head]=v;head=(head+1)%samples.size();}
 };
 std::array<Ring,2> echoes,modulation;
 std::array<std::array<Ring,4>,2> combs;
 std::array<std::array<Ring,2>,2> allpasses;
 std::array<float,2> low{},high{},highInput{};
 std::array<Ramp,11> smooth;Ramp delayTime;
 double sr=48000,phase=0,flutter=0;float noiseEnvelope=0;uint32_t noise=0x735ac12u;int ramp=960;
public:
 void prepare(double rate){sr=std::isfinite(rate)?std::clamp(rate,8000.0,384000.0):48000.0;ramp=std::max(1,int(sr*.02));phase=flutter=0;noiseEnvelope=0;noise=0x735ac12u;low.fill(0);high.fill(0);highInput.fill(0);
  const std::array<double,4> times{.0297,.0371,.0411,.0437};
  for(int ch=0;ch<2;++ch){echoes[size_t(ch)].prepare(size_t(sr*3.1)+4);modulation[size_t(ch)].prepare(size_t(sr*.08)+4);for(int i=0;i<4;++i)combs[size_t(ch)][size_t(i)].prepare(size_t(sr*(times[size_t(i)]+ch*.0011))+2);for(int i=0;i<2;++i)allpasses[size_t(ch)][size_t(i)].prepare(size_t(sr*(i==0?.005:.0017))+2);}
  delayTime={};delayTime.value=delayTime.target=float(sr*.5);smooth={};smooth[5].value=smooth[5].target=20000;smooth[6].value=smooth[6].target=20;smooth[7].value=smooth[7].target=1;
 }
 void set(const EffectSettings& p){std::array<float,11> v{p.enabled?1.f:0.f,p.reverb,p.delay,p.chorus,p.saturation,p.lowpass,p.highpass,p.width,p.wow,p.vinyl,0};for(size_t i=0;i<v.size();++i)smooth[i].set(v[i],ramp);}
 void process(float* left,float* right,int count,double bpm){
  bpm=std::isfinite(bpm)?std::clamp(bpm,20.0,999.0):120;delayTime.set(float(sr*60/bpm),ramp);
  if(smooth[0].value==0&&smooth[0].target==0&&delayTime.remaining==0&&std::all_of(smooth.begin(),smooth.end(),[](const auto& value){return value.remaining==0;}))return;
  for(int frame=0;frame<count;++frame){float delayFrames=delayTime.next(),wet=smooth[0].next();std::array<float,10> p{};for(int i=1;i<11;++i)p[size_t(i-1)]=smooth[size_t(i)].next();
   if(wet==0&&smooth[0].target==0)continue;
   float rawL=left[frame],rawR=right[frame];std::array<float,2> x{rawL,rawR};
   float lp=float(std::exp(-2*pi*std::clamp(double(p[4]),30.0,sr*.45)/sr));float hp=float(std::exp(-2*pi*std::clamp(double(p[5]),10.0,sr*.4)/sr));
   noiseEnvelope=std::max(std::min(1.f,(std::abs(rawL)+std::abs(rawR))*10),noiseEnvelope*.9995f);
   for(int ch=0;ch<2;++ch){size_t c=size_t(ch);float dry=x[c];high[c]=hp*(high[c]+dry-highInput[c]);highInput[c]=dry;low[c]=(1-lp)*high[c]+lp*low[c];float v=low[c];
    float drive=1+12*p[3];float saturated=std::tanh(drive*v)/std::tanh(drive);v=v*(1-p[3])+saturated*p[3];
    auto& mod=modulation[c];mod.push(v);double wobble=std::sin(phase*.37)+.22*std::sin(flutter);double wowDelay=sr*(.004+.0015*wobble);float wowed=mod.read(wowDelay);v=v*(1-p[7])+wowed*p[7];
    double chorusDelay=sr*(.016+.005*std::sin(phase+ch*pi*.5));float chorused=mod.read(chorusDelay);v=v*(1-.5f*p[2])+chorused*.5f*p[2];
    auto& echo=echoes[c];float delayed=echo.read(delayFrames);echo.push(v+delayed*.42f);v+=delayed*p[1]*.6f;
    float rv=0;for(auto& ring:combs[c]){float old=ring.read(double(ring.samples.size()-2));ring.push(v+old*.80f);rv+=old*.25f;}
    for(auto& ring:allpasses[c]){float old=ring.read(double(ring.samples.size()-2));float next=rv+old*.5f;ring.push(next);rv=old-next*.5f;}
    v=v*(1-.28f*p[0])+rv*p[0]*.7f;
    noise=1664525u*noise+1013904223u;float hiss=float(int32_t(noise))/2147483648.f;
    // Sparse crackles plus low-level hiss; gated by the source envelope.
    float crackle=(noise&8191u)==0?float((noise>>16)&255)/255.f-.5f:0;
    x[c]=v+(hiss*.004f+crackle*.07f)*p[8]*noiseEnvelope;
   }
   float mid=(x[0]+x[1])*.5f,side=(x[0]-x[1])*.5f*p[6];
   left[frame]=rawL*(1-wet)+(mid+side)*wet;right[frame]=rawR*(1-wet)+(mid-side)*wet;
   phase+=2*pi*.25/sr;flutter+=2*pi*6.7/sr;if(phase>200*pi)phase-=200*pi;if(flutter>2*pi)flutter-=2*pi;
  }
 }
};
}
