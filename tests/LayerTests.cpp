#include "Music/Composition.h"
#include "Export/Midi.h"
#include <iostream>
#include <stdexcept>
void check(bool b){if(!b)throw std::runtime_error("layer test failed");}
int main(){try{
 for(int style=0;style<9;++style)for(int key=0;key<12;++key)for(int bars:{8,16})for(int scale:{0,1}){
  wee::Settings s;s.style=style;s.key=key;s.bars=bars;s.scale=scale;auto a=wee::generateArrangement(s),b=wee::generateArrangement(s);
  for(int i=0;i<8;++i){check(!a.layers[size_t(i)].notes.empty());check(a.layers[size_t(i)].notes==b.layers[size_t(i)].notes);for(auto&n:a.layers[size_t(i)].notes){check(n.beat>=0&&n.length>0&&n.beat+n.length<=bars*4+1.0e-8);int relative=(n.pitch-key)%12;bool inScale=false;for(int d=0;d<7;++d)inScale|=(wee::pitch(s,d)%12-key+12)%12==relative;check(inScale);}}
 }
 auto a=wee::generateArrangement({});std::array<bool,8> include;include.fill(true);auto file=wee::midi(a,include);check(file[9]==1&&file[11]==9);size_t at=14;
 auto u32=[&](){check(at+4<=file.size());uint32_t n=0;for(int i=0;i<4;++i)n=(n<<8)|file[at++];return n;};
 for(int track=0;track<9;++track){check(u32()==0x4d54726bu);uint32_t size=u32();size_t end=at+size;check(end<=file.size());int ticks=0,ons=0,offs=0;auto vlq=[&](){int value=0,v;do{check(at<end);v=file[at++];value=(value<<7)|(v&127);}while(v&128);return value;};
  while(at<end){ticks+=vlq();int status=file[at++];if(status==255){int type=file[at++];int len=vlq();if(type==47)check(ticks==a.context.bars*4*960);at+=size_t(len);}else{check((status&15)==track-1);if((status&240)==144)++ons;else if((status&240)==128)++offs;else check(false);at+=2;}}
  check(at==end&&ons==offs);check(ons==(track==0?0:int(a.layers[size_t(track-1)].notes.size())));
 }
 check(at==file.size());std::cout<<"PASS: 432 harmonic contexts, eight layers, seeded determinism and independent format-1 MIDI parsing\n";
 }catch(const std::exception&e){std::cerr<<e.what();return 1;}}
