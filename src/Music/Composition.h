#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>
namespace wee {
struct Note { double beat=0, length=0.5; int pitch=60, velocity=90; bool operator==(const Note&) const = default; };
struct Settings { int bars=8, key=9, scale=0, style=1, mood=0; uint32_t seed=1; double bpm=120; };
struct Composition { Settings settings; std::vector<Note> notes; };
inline Composition generate(Settings s) {
 s.bars=s.bars==16?16:8; s.key=std::clamp(s.key,0,11); s.bpm=std::clamp(s.bpm,40.0,240.0);
 Composition c{s,{}}; uint32_t rng=s.seed; auto next=[&](){rng=1664525u*rng+1013904223u;return rng;};
 const std::array<int,7> minor{0,2,3,5,7,8,10}, major{0,2,4,5,7,9,11}; auto scale=s.scale==0?minor:major;
 const std::array<int,4> progression{0,5,2,6}; int previous=0;
 for(int bar=0;bar<s.bars;++bar) {
  int root=progression[(bar/2)%4];
  for(int t=0;t<8;++t) {
   auto v=next(); if((v%100)<(s.style==1?40u:20u) && t!=0)continue;
   int degree=t%4==0?root:std::clamp(previous+int(v%3)-1,0,9);
   if(t==7 && bar%4==3)degree=0;
   previous=degree;
   int pitch=60+s.key+scale[degree%7]+12*(degree/7);
   double start=bar*4+t*0.5+(t==0?0:(int(next()%9)-4)*0.002);
   double len= s.style==2?0.7:0.4;
   c.notes.push_back({std::max(0.0,start),std::min(len,s.bars*4-start),pitch,75+int(next()%30)});
  }
 }
 std::sort(c.notes.begin(),c.notes.end(),[](auto&a,auto&b){return a.beat<b.beat;}); return c;
}
}
