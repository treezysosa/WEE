#pragma once
#include <array>
namespace wee {
struct FactoryPreset{const char* name;int style,key,scale,instrument;float bpm;int backing;};
inline constexpr std::array<FactoryPreset,6> factoryPresets{{
 {"Moonlit Piano",1,9,0,0,120,2},{"Indie Daydream",2,0,1,1,104,6},
 {"Soul Morning",8,5,1,7,92,2},{"Picked Memories",3,2,0,2,96,6},
 {"Glass Atmosphere",6,7,0,5,80,5},{"Pop Spark",0,0,1,1,124,2}
}};
}
