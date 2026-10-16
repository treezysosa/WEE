#pragma once
#include "Composition.h"
namespace wee {
// One gesture edits a draft; commit only a valid, complete layer.
inline bool validNotes(const Composition& c){
 if(c.notes.size()>2048)return false;
 for(const auto& n:c.notes)if(!std::isfinite(n.beat)||!std::isfinite(n.length)||n.beat<0||n.length<0.001||n.beat+n.length>c.settings.bars*4+1.e-8||n.pitch<0||n.pitch>127||n.velocity<1||n.velocity>127)return false;
 return true;
}
inline double snapBeat(double beat,double grid){return std::round(beat/grid)*grid;}
inline Note editNote(Note n,double beat,int pitch,double length,double grid,int bars){
 n.beat=std::clamp(snapBeat(beat,grid),0.0,bars*4-grid);
 n.pitch=std::clamp(pitch,0,127);
 n.length=std::clamp(snapBeat(length,grid),grid,bars*4-n.beat);return n;
}
}
