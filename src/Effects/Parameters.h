#pragma once
#include <array>
namespace wee {
struct EffectParameter {const char* id;const char* label;float low,high,value;int kind,section;};
// kind: 0 float, 1 bool, 2 integer, 3 WAV depth choice. Stable IDs are appended to old parameters.
inline constexpr std::array<EffectParameter,24> effectParameters{{
 {"fx_enable","Live effects",0,1,0,1,0}, {"fx_reverb","Reverb",0,1,0,0,0},
 {"fx_delay","Quarter-note delay",0,1,0,0,0}, {"fx_chorus","Chorus",0,1,0,0,0},
 {"fx_saturation","Saturation",0,1,0,0,0}, {"fx_lowpass","Low-pass (Hz)",30,20000,20000,0,0},
 {"fx_highpass","High-pass (Hz)",20,4000,20,0,0}, {"fx_width","Stereo width",0,2,1,0,0},
 {"fx_wow","Wow / flutter",0,1,0,0,0}, {"fx_vinyl","Vinyl noise",0,1,0,0,0},
 {"flip_export","Apply FLIP to WAV",0,1,0,1,1}, {"flip_half","Half-time",0,1,1,1,1},
 {"flip_reverse","Reverse",0,1,0,1,1}, {"flip_pitch","Transpose (semitones)",-24,24,0,2,1},
 {"flip_tape","Tape warmth",0,1,.3f,0,1}, {"flip_wow","Tape wow / flutter",0,1,.15f,0,1},
 {"flip_lowpass","FLIP low-pass (Hz)",200,20000,9000,0,1}, {"flip_reverb","FLIP reverb",0,1,.2f,0,1},
 {"flip_vintage","Vintage character",0,1,.2f,0,1},
 {"wav_depth","WAV format",0,2,1,3,2}, {"wav_seamless","Seamless boundaries",0,1,1,1,2},
 {"wav_tail","Separate tail file",0,1,0,1,2}, {"wav_tailseconds","Tail seconds",1,30,8,0,2},
 {"wav_protect","Peak protection",0,1,1,1,2}
}};
}
