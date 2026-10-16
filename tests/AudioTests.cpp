#include "Effects/EffectChain.h"
#include "Effects/Flip.h"
#include "Export/Wav.h"
#include <iostream>
#include <stdexcept>
#include <iterator>
void check(bool b,const char*m){if(!b)throw std::runtime_error(m);}
double energy(const wee::StereoAudio&a){double n=0;for(auto&c:a.channels)for(float x:c){check(std::isfinite(x)&&std::abs(x)<20,"finite bounded DSP");n+=double(x)*x;}return n;}
double difference(const wee::StereoAudio&a,const wee::StereoAudio&b){double n=0;for(int c=0;c<2;++c)for(size_t i=0;i<a.size();++i)n+=std::abs(a.channels[size_t(c)][i]-b.channels[size_t(c)][i]);return n;}
wee::StereoAudio process(wee::StereoAudio input,wee::EffectSettings p,int chunk=32000){wee::EffectChain effects;effects.prepare(8000);effects.set(p);for(size_t at=0;at<input.size();at+=size_t(chunk))effects.process(input.channels[0].data()+at,input.channels[1].data()+at,int(std::min(size_t(chunk),input.size()-at)),120);return input;}
int main(){try{
 wee::StereoAudio source;source.resize(32000);for(size_t i=0;i<source.size();++i){source.channels[0][i]=float(.15*std::sin(2*wee::pi*173*double(i)/8000));source.channels[1][i]=float(.11*std::cos(2*wee::pi*499*double(i)/8000));}source.channels[0][1000]+=.5f;
 wee::EffectSettings bypass;check(difference(source,process(source,bypass))==0,"exact DSP bypass");wee::EffectSettings base;base.enabled=true;auto dry=process(source,base);
 for(int kind=0;kind<9;++kind){auto p=base;switch(kind){case 0:p.reverb=1;break;case 1:p.delay=1;break;case 2:p.chorus=1;break;case 3:p.saturation=1;break;case 4:p.lowpass=200;break;case 5:p.highpass=2000;break;case 6:p.width=0;break;case 7:p.wow=1;break;case 8:p.vinyl=1;break;}auto wet=process(source,p);check(energy(wet)>0,"effect audible");check(difference(wet,dry)>.001,"every effect changes real samples");check(difference(wet,process(source,p,127))==0,"DSP block partition invariance");}
 for(double rate:{44100.0,48000.0,96000.0}){auto input=source;wee::EffectChain fx;fx.prepare(rate);auto p=base;p.reverb=.8f;p.delay=.7f;p.chorus=.6f;p.saturation=.4f;p.wow=.5f;p.vinyl=.2f;fx.set(p);fx.process(input.channels[0].data(),input.channels[1].data(),16000,120);fx.process(input.channels[0].data()+16000,input.channels[1].data()+16000,16000,137);check(energy(input)>0,"DSP rates and tempo changes");}
 wee::StereoAudio sine;sine.resize(8000);for(size_t i=0;i<sine.size();++i)for(auto& c:sine.channels)c[i]=float(.5*std::sin(2*wee::pi*200*double(i)/8000));
 wee::FlipSettings flip;flip.half=false;check(difference(sine,wee::transform(sine,8000,flip))==0,"identity transform");flip.reverse=true;auto reverse=wee::transform(sine,8000,flip);for(size_t i=0;i<sine.size();++i)check(reverse.channels[0][i]==sine.channels[0][sine.size()-1-i],"true audio reverse");
 flip.reverse=false;flip.half=true;auto half=wee::transform(sine,8000,flip);auto crossings=[](const wee::StereoAudio&a){int n=0;for(size_t i=1;i<a.size();++i)if(a.channels[0][i-1]<=0&&a.channels[0][i]>0)++n;return n;};check(std::abs(crossings(half)-100)<=1,"half-time pitch and duration");flip.half=false;flip.transpose=12;auto up=wee::transform(sine,8000,flip);check(std::abs(crossings(up)-400)<=2,"pitch transpose preserves length");
 auto count=up.size();wee::softenBoundary(up,8000);check(up.size()==count&&up.channels[0].front()==0&&up.channels[0].back()==0,"seam conditioning preserves exact duration");
 std::atomic<bool> cancelled{true};bool stopped=false;try{wee::transform(sine,8000,flip,&cancelled);}catch(...){stopped=true;}check(stopped,"transform cancellation");
 auto folder=std::filesystem::current_path()/"test-output";std::filesystem::create_directories(folder);sine.channels[0][0]=1.25f;sine.channels[1][0]=-1.25f;
 for(int depth=0;depth<3;++depth){auto file=folder/("format-"+std::to_string(depth)+".wav");wee::writeWav(file,sine,8000,wee::WavDepth(depth));std::ifstream in(file,std::ios::binary);std::vector<uint8_t>b((std::istreambuf_iterator<char>(in)),{});auto le=[&](size_t at,int n){check(at+size_t(n)<=b.size(),"WAV bounds");uint32_t v=0;for(int i=0;i<n;++i)v|=uint32_t(b[at+size_t(i)])<<(8*i);return v;};
  check(std::string(b.begin(),b.begin()+4)=="RIFF"&&le(4,4)==b.size()-8,"RIFF size");size_t at=12;bool fmt=false,data=false,fact=false;int bytes=depth==0?2:depth==1?3:4;
  while(at+8<=b.size()){std::string tag(b.begin()+std::ptrdiff_t(at),b.begin()+std::ptrdiff_t(at+4));uint32_t length=le(at+4,4);size_t p=at+8;check(p+length<=b.size(),"chunk bounds");if(tag=="fmt "){fmt=true;check(le(p,2)==uint32_t(depth==2?3:1)&&le(p+2,2)==2&&le(p+4,4)==8000&&le(p+14,2)==uint32_t(bytes*8),"WAV format/rate/stereo/depth");}if(tag=="fact"){fact=true;check(le(p,4)==8000,"float fact length");}if(tag=="data"){data=true;check(length==8000u*2u*uint32_t(bytes),"exact WAV frames");if(depth==2)check(std::bit_cast<float>(le(p,4))==1.25f,"true IEEE float headroom");else check(le(p,bytes)==uint32_t(depth==0?32767:8388607),"PCM clipping bounds");}at=p+length+(length&1u);}
  check(fmt&&data&&(depth!=2||fact)&&at==b.size(),"standards-compliant WAV chunks");
 }
 std::cout<<"PASS: nine DSP effects, bypass, partition invariance, reverse/half/pitch, cancellation and independent WAV PCM16/24/IEEE-float parser\n";
 }catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}}
