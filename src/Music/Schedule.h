#pragma once
#include "Composition.h"
namespace wee {
struct ScheduledEvent{double beat;int pitch,velocity;};
// Separate streams preserve note-off priority even when distinct beats round to one sample.
struct LayerSchedule{std::vector<ScheduledEvent> on,off;};
inline LayerSchedule makeSchedule(const Composition& c){
 LayerSchedule result;result.on.reserve(c.notes.size());result.off.reserve(c.notes.size());double loop=c.settings.bars*4;
 for(auto n:c.notes){result.on.push_back({n.beat,n.pitch,n.velocity});result.off.push_back({std::fmod(n.beat+n.length,loop),n.pitch,0});}
 auto less=[](const auto& a,const auto& b){return a.beat<b.beat;};std::stable_sort(result.on.begin(),result.on.end(),less);std::stable_sort(result.off.begin(),result.off.end(),less);return result;
}
template<class Emit> inline void scheduleRange(const std::vector<ScheduledEvent>& stream,double begin,double end,double loop,Emit&& emit){
 if(stream.empty()||end<=begin)return;
 double offset=std::floor(begin/loop)*loop;
 auto it=std::lower_bound(stream.begin(),stream.end(),begin,[offset](const auto& event,double beat){return offset+event.beat<beat;});
 while(offset<end){for(;it!=stream.end();++it){double at=offset+it->beat;if(at>=end)break;if(at>=begin)emit(*it,at);}offset+=loop;it=stream.begin();}
}
}
