#pragma once
#include "../Effects/Flip.h"
#include <bit>
#include <filesystem>
#include <fstream>
#include <limits>
namespace wee {
enum class WavDepth {pcm16,pcm24,float32};
inline void little(std::ostream& stream,uint32_t value,int bytes){for(int i=0;i<bytes;++i)stream.put(char(uint8_t(value>>(8*i))));}
inline void writeWav(const std::filesystem::path& path,const StereoAudio& audio,double rate,WavDepth depth,float gain=1,const std::atomic<bool>* cancel=nullptr){
 if(audio.channels[1].size()!=audio.size()||!std::isfinite(rate)||rate<8000||rate>384000)throw std::runtime_error("Invalid WAV audio or sample rate");
 int bits=depth==WavDepth::pcm16?16:depth==WavDepth::pcm24?24:32;bool floating=depth==WavDepth::float32;uint64_t dataBytes=uint64_t(audio.size())*2*uint64_t(bits/8);uint32_t extra=floating?14:0;
 if(dataBytes>std::numeric_limits<uint32_t>::max()-36-extra)throw std::runtime_error("WAV exceeds RIFF size limit");
 std::ofstream out(path,std::ios::binary|std::ios::trunc);if(!out)throw std::runtime_error("Cannot open WAV destination");
 out.write("RIFF",4);little(out,uint32_t(dataBytes)+36+extra,4);out.write("WAVEfmt ",8);little(out,floating?18:16,4);little(out,floating?3:1,2);little(out,2,2);uint32_t sampleRate=uint32_t(std::lround(rate));little(out,sampleRate,4);little(out,sampleRate*2*uint32_t(bits/8),4);little(out,uint32_t(2*bits/8),2);little(out,uint32_t(bits),2);
 if(floating){little(out,0,2);out.write("fact",4);little(out,4,4);little(out,uint32_t(audio.size()),4);}
 out.write("data",4);little(out,uint32_t(dataBytes),4);
 for(size_t i=0;i<audio.size();++i){if((i&4095u)==0&&cancelled(cancel))throw std::runtime_error("Cancelled");for(int ch=0;ch<2;++ch){float value=audio.channels[size_t(ch)][i]*gain;if(!std::isfinite(value))throw std::runtime_error("Non-finite WAV sample");if(floating)little(out,std::bit_cast<uint32_t>(value),4);else{double scale=bits==16?32768.0:8388608.0;int32_t integer=int32_t(std::clamp(std::round(double(value)*scale),-scale,scale-1));little(out,uint32_t(integer),bits/8);}}}
 out.flush();if(!out)throw std::runtime_error("WAV write failed (disk full or permission denied)");
}
}
