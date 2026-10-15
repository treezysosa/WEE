#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>
namespace wee {
inline constexpr int layerCount = 8;
inline constexpr std::array<const char*, 9> styleNames{"Pop", "Dark Pop", "Indie Pop", "Guitar", "Piano", "Strings", "Pads", "Vocal", "Soul"};
inline constexpr std::array<const char*, 8> instrumentNames{"Piano", "Electric Piano", "Acoustic-style Guitar", "Clean Electric Guitar", "String Ensemble", "Analog Pad", "Vocal Synth", "Soul Keys"};
inline constexpr std::array<const char*, 8> layerNames{"Lead Melody", "Counter Melody", "Chords", "Piano", "Guitar", "Strings", "Pad", "Vocal Texture"};
inline constexpr std::array<int, 8> defaultInstruments{0, 1, 7, 0, 2, 4, 5, 6};
struct Note { double beat=0, length=0.5; int pitch=60, velocity=90; bool operator==(const Note&) const = default; };
struct Settings { int bars=8, key=9, scale=0, style=1, mood=0; uint32_t seed=1; double bpm=120; bool operator==(const Settings&) const = default; };
struct Composition { Settings settings; std::vector<Note> notes; };
struct Arrangement { Settings context; std::array<Composition, layerCount> layers; };
struct Random { uint32_t state; uint32_t next(){state=1664525u*state+1013904223u;return state;} };
inline Settings sanitise(Settings s){s.bars=s.bars==16?16:8;s.key=std::clamp(s.key,0,11);s.scale=std::clamp(s.scale,0,1);s.style=std::clamp(s.style,0,8);s.bpm=std::isfinite(s.bpm)?std::clamp(s.bpm,40.0,240.0):120;return s;}
inline int pitch(const Settings& s,int degree,int base=60){
 constexpr std::array<int,7> minor{0,2,3,5,7,8,10},major{0,2,4,5,7,9,11};
 int oct=degree/7,index=degree%7;if(index<0){index+=7;--oct;}
 return std::clamp(base+s.key+(s.scale==0?minor:major)[size_t(index)]+12*oct,0,127);
}
inline int chord(const Settings& s,int bar){constexpr std::array<int,4> pop{0,5,2,6},soul{0,5,1,4};return (s.style==8?soul:pop)[size_t((bar/2)%4)];}
inline void add(Composition& c,double beat,double length,int note,int velocity){
 beat=std::max(0.0,beat);length=std::min(length,c.settings.bars*4-beat);
 if(length>0.001)c.notes.push_back({beat,length,std::clamp(note,0,127),std::clamp(velocity,1,127)});
}
inline void sort(Composition& c){std::stable_sort(c.notes.begin(),c.notes.end(),[](auto&a,auto&b){return a.beat<b.beat;});}
inline Composition generate(Settings input){
 auto s=sanitise(input);Composition c{s,{}};Random rng{s.seed};
 std::array<int,8> motif{};for(auto&d:motif)d=int(rng.next()%5);
 double grid=s.style==4?0.25:s.style==5?1.0:s.style==6?2.0:0.5;
 int steps=int(4/grid),previous=0;
 for(int bar=0;bar<s.bars;++bar)for(int t=0;t<steps;++t){
  auto v=rng.next();int rest=s.style==1?42:s.style==7?48:s.style==2?28:18;
  if((int(v%100)<rest && t!=0)||(s.style==7 && bar%2==1 && t>4))continue;
  int root=chord(s,bar);int degree=t%4==0?root:motif[size_t((t+bar%2*4)%8)];
  if(bar%4==3 && t==steps-1)degree=bar%8==7?0:root;
  if(bar%4==2 && t%4!=0)degree=std::clamp(previous+int(v%3)-1,0,9);
  previous=degree;
  double timing=t==0?0:(int(rng.next()%9)-4)*0.002;
  if(s.style==8 && t%2)timing+=0.065;
  double len=grid*(s.style==5||s.style==6?0.97:s.style==2?1.2:0.78);
  int base=s.style==3?48:60;
  add(c,bar*4+t*grid+timing,len,pitch(s,degree,base),68+int(rng.next()%35));
 }
 sort(c);return c;
}
inline Composition generateLayer(Settings input,int layer){
 auto context=sanitise(input);auto s=context;s.seed^=uint32_t(layer)*0x9e3779b9u;
 if(layer==0)return generate(s);
 Composition c{s,{}};Random rng{s.seed};
 if(layer==1){auto lead=generate(s);for(auto n:lead.notes)if(int(n.beat/4)%2==1 && rng.next()%100<60){n.pitch=std::max(36,n.pitch-12);n.beat+=0.25;n.length*=0.8;add(c,n.beat,n.length,n.pitch,n.velocity-12);}}
 else if(layer==2||layer==5||layer==6){
  int voices=context.style==8?5:3;std::array<int,5> previous{48,55,60,64,67};if(rng.next()%2)for(auto& n:previous)n+=12;
  for(int bar=0;bar<s.bars;bar+=2){int root=chord(context,bar);for(int v=0;v<voices;++v){
   int note=pitch(context,root+v*2,48);while(note-previous[size_t(v)]>6)note-=12;while(previous[size_t(v)]-note>6)note+=12;
   previous[size_t(v)]=note;double offset=layer==2?0.008*(rng.next()%5):0;
   add(c,bar*4+offset,layer==6?7.7+(rng.next()%25)*0.01:layer==5?7.4+(rng.next()%30)*0.01:7.1+(rng.next()%40)*0.01,note+(layer==5?12:0),layer==6?50+int(rng.next()%15):layer==5?61+int(rng.next()%16):72+int(rng.next()%15));
  }}
 }else if(layer==3||layer==4){
  constexpr std::array<int,8> arpeggio{0,2,4,6,4,2,0,2},picking{0,4,2,4,0,2,4,2};
  for(int bar=0;bar<s.bars;++bar)for(int t=0;t<8;++t){if(layer==4 && rng.next()%100<18)continue;
   int degree=chord(context,bar)+(layer==3?arpeggio:picking)[size_t(t)];
   double strum=layer==4?(rng.next()%12)*0.001:0;
   add(c,bar*4+t*0.5+strum,layer==4?0.42:0.55,pitch(context,degree,48),64+int(rng.next()%28));
  }
 }else{for(int bar=0;bar<s.bars;bar+=2){int d=chord(context,bar)+int(rng.next()%2)*2;add(c,bar*4+1,2.5,pitch(context,d,60),55+int(rng.next()%20));}}
 sort(c);return c;
}
inline Arrangement generateArrangement(Settings s){Arrangement a;a.context=sanitise(s);for(int i=0;i<layerCount;++i)a.layers[size_t(i)]=generateLayer(a.context,i);return a;}
inline bool sameHarmony(const Settings&a,const Settings&b){return a.key==b.key&&a.scale==b.scale&&a.bars==b.bars&&a.style==b.style;}
}
