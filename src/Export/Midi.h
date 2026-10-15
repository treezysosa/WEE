#pragma once
#include "../Music/Composition.h"
#include <fstream>
#include <string>
namespace wee {
inline void be(std::vector<uint8_t>& b,uint32_t n,int count){for(int i=count-1;i>=0;--i)b.push_back(uint8_t(n>>(8*i)));}
inline void vlq(std::vector<uint8_t>& b,uint32_t n){uint8_t a[5];int i=0;a[i++]=n&127;while((n>>=7))a[i++]=uint8_t((n&127)|128);while(i)b.push_back(a[--i]);}
inline std::vector<uint8_t> midiTrack(const Composition& c,int channel,bool metadata,const char* name){
 struct Event{int tick;std::vector<uint8_t> bytes;};std::vector<Event> events;
 if(metadata){int tempo=int(std::lround(60000000/std::clamp(c.settings.bpm,20.0,999.0)));events.push_back({0,{255,81,3,uint8_t(tempo>>16),uint8_t(tempo>>8),uint8_t(tempo)}});events.push_back({0,{255,88,4,4,2,24,8}});}
 for(auto& n:c.notes){events.push_back({int(std::lround(n.beat*960)),{uint8_t(144+channel),uint8_t(n.pitch),uint8_t(n.velocity)}});events.push_back({int(std::lround((n.beat+n.length)*960)),{uint8_t(128+channel),uint8_t(n.pitch),0}});}
 std::stable_sort(events.begin(),events.end(),[](auto&a,auto&b){return a.tick<b.tick||(a.tick==b.tick&&a.bytes[0]<b.bytes[0]);});
 std::vector<uint8_t> track;if(name){vlq(track,0);track.insert(track.end(),{255,3});std::string text(name);vlq(track,uint32_t(text.size()));track.insert(track.end(),text.begin(),text.end());}
 int last=0;for(auto& e:events){vlq(track,uint32_t(e.tick-last));track.insert(track.end(),e.bytes.begin(),e.bytes.end());last=e.tick;}
 vlq(track,uint32_t(c.settings.bars*4*960-last));track.insert(track.end(),{255,47,0});return track;
}
inline void appendTrack(std::vector<uint8_t>& out,const std::vector<uint8_t>& track){out.insert(out.end(),{'M','T','r','k'});be(out,uint32_t(track.size()),4);out.insert(out.end(),track.begin(),track.end());}
inline std::vector<uint8_t> midi(const Composition& c){std::vector<uint8_t> out{'M','T','h','d'};be(out,6,4);be(out,0,2);be(out,1,2);be(out,960,2);appendTrack(out,midiTrack(c,0,true,nullptr));return out;}
inline std::vector<uint8_t> midi(const Arrangement& a,const std::array<bool,8>& include){
 std::vector<uint8_t> out{'M','T','h','d'};be(out,6,4);be(out,1,2);int count=1;for(bool on:include)if(on)++count;be(out,uint32_t(count),2);be(out,960,2);
 appendTrack(out,midiTrack(Composition{a.context,{}},0,true,"WEE Tempo"));
 for(int i=0;i<8;++i)if(include[size_t(i)]){auto c=a.layers[size_t(i)];c.settings.bars=a.context.bars;appendTrack(out,midiTrack(c,i,false,layerNames[size_t(i)]));}
 return out;
}
inline bool saveMidi(const Composition& c,const std::string& path){auto b=midi(c);std::ofstream f(path,std::ios::binary);f.write(reinterpret_cast<const char*>(b.data()),std::streamsize(b.size()));return bool(f);}
}
