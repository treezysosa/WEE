#include "Music/Schedule.h"
#include <chrono>
#include <iostream>
#include <tuple>
#include <stdexcept>
using Hit=std::tuple<int,int,int,int>;
static void check(bool b){if(!b)throw std::runtime_error("schedule reference mismatch");}
static std::vector<Hit> collect(const wee::Composition& c,double begin,double step,int count,bool optimized){
 std::vector<Hit> hits;double loop=c.settings.bars*4,end=begin+count*step;
 for(int off=1;off>=0;--off){auto emit=[&](int pitch,int velocity,double at){hits.emplace_back(std::clamp(int(std::lround((at-begin)/step)),0,count-1),off,pitch,velocity);};
 if(optimized){auto schedule=wee::makeSchedule(c);wee::scheduleRange(off?schedule.off:schedule.on,begin,end,loop,[&](auto e,double at){emit(e.pitch,e.velocity,at);});}
 else for(auto n:c.notes){double event=n.beat+(off?n.length:0);double at=event+std::ceil((begin-event)/loop)*loop;while(at<end){emit(n.pitch,off?0:n.velocity,at);at+=loop;}}
 }
 // MIDI orders by sample; compare the event multiset with off priority.
 std::sort(hits.begin(),hits.end());return hits;
}
int main(){try{
 for(int bars:{8,16}){wee::Settings s;s.bars=bars;auto c=wee::generate(s);c.notes.push_back({0,1,60,100});c.notes.push_back({double(bars*4-1),1,60,80});
 for(double rate:{8000.,44100.,48000.,96000.})for(double bpm:{40.,120.,137.,240.,999.})for(int count:{1,64,256,8192})for(double begin:{-65.,-32.,-.5,0.,.008,1.,31.99,32.,64.,123456.})check(collect(c,begin,bpm/(60*rate),count,true)==collect(c,begin,bpm/(60*rate),count,false));
 }
 // Reproduce an exact event at a nonzero loop offset without subtractive cancellation.
 wee::Composition boundary;boundary.notes={{.008,.5,69,100}};auto timeline=wee::makeSchedule(boundary);int n=0;wee::scheduleRange(timeline.on,32+.008,32+.009,32,[&](auto,double){++n;});check(n==1);
 // A sub-sample note-off still precedes a rounded note-on in the processor's two streams.
 wee::Composition dense;for(int i=0;i<2048;++i)dense.notes.push_back({double(i)/64,.01,36+i%60,90});auto schedule=wee::makeSchedule(dense);size_t emitted=0;auto start=std::chrono::steady_clock::now();
 for(int block=0;block<100000;++block){double begin=block*.002;for(auto* stream:{&schedule.off,&schedule.on})wee::scheduleRange(*stream,begin,begin+.002,32,[&](auto,double){++emitted;});}
 auto ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();check(emitted>0&&emitted<1000000);
 std::cout<<"PASS: scheduler equals independent scan over rates/tempos/negative PPQ/wraps/blocks; 100k dense blocks "<<ms<<" ms, "<<emitted<<" emitted events (no timing gate)\n";
 }catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}}
