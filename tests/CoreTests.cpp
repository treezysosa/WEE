#include "Music/Composition.h"
#include "Export/Midi.h"
#include <iostream>
#include <stdexcept>
void check(bool x){if(!x)throw std::runtime_error("test failed");}
int main(){try{for(int bars:{8,16})for(int key=0;key<12;++key){wee::Settings s;s.bars=bars;s.key=key;auto a=wee::generate(s);check(a.notes==wee::generate(s).notes);s.seed++;check(a.notes!=wee::generate(s).notes);for(auto&n:a.notes){check(n.beat>=0&&n.beat+n.length<=bars*4);check(n.pitch>=0&&n.pitch<128);}auto b=wee::midi(a);check(b.size()>22&&b[0]=='M'&&b[14]=='M');size_t at=22;int tick=0,ons=0,offs=0;auto vlq=[&](){int n=0,v;do{check(at<b.size());v=b[at++];n=(n<<7)|(v&127);}while(v&128);return n;};while(at<b.size()){tick+=vlq();int status=b[at++];if(status==255){int type=b[at++];int len=vlq();if(type==47)check(tick==bars*4*960);at+=len;}else{check(status==144||status==128);check(at+2<=b.size());if(status==144)++ons;else ++offs;check(b[at]<128&&b[at+1]<128);at+=2;}}check(ons==int(a.notes.size())&&offs==ons);}std::cout<<"PASS: deterministic generation, bounds, independent SMF event parsing and exact duration\n";}catch(const std::exception&e){std::cerr<<e.what();return 1;}}
