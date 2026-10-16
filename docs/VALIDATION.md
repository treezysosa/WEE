# Phase 3 validation — passed 2026-10-16

- Build: https://github.com/treezysosa/WEE/actions/runs/38829866854
- Compiled source: 269d7a3d1c10c333299777717d9e2098dff5dfd3
- Toolchain: Windows Server 2022 GitHub runner, Visual Studio 2022 x64 Release, C++20, JUCE 8.0.6. Plugin version 0.3.0.
- VST3 instrument and standalone executable compiled successfully.
- CTest: 4/4 passed — composition_midi, layer_harmony_midi, dsp_flip_wav, processor_audio_state.
- Core/layer checks: deterministic generation, 432 harmonic contexts, scale/bounds and independent single/multitrack MIDI parsing.
- DSP checks: all nine effects alter real stereo samples; exact bypass; block partition invariance; 44.1/48/96 kHz and tempo changes; true sample reverse; half-speed and octave transpose; constant-duration transforms; boundary conditioning; cancellation; independent PCM16/24/IEEE float32 RIFF/fact/chunk parsing. Python wave independently imports both PCM fixtures.
- Integrated renderer: actual instruments/effects; exact 8/16-bar frame rounding (including fractional 137 BPM and 48 kHz); separate tail counts/energy; transformed FLIP audio; independent JUCE reader for all three WAV formats; V1/V2/V3 state migration; effect values recalled against their actual normalized parameter values; cached A/B playback/stop/edit invalidation; background stem files plus tails; cancellation; live-state preservation; same-pitch boundary retrigger; explicit unsupported-meter refusal without changing music.
- Existing audio/mixer/lock/editor regression checks continue to pass.
- Packaged installer installed the bundle in C:\Program Files\Common Files\VST3\WEE.vst3; binary hash matched; replacement created a timestamped backup.
- Download: https://github.com/treezysosa/WEE/actions/runs/38829866854/artifacts/12017081048
- ZIP SHA-256: 3becad7cdc849883f91b38e6d9bb073db3c0f799d0096df535d899665f761330
- Artifact retained until 2026-11-15. Package contains VST3, standalone, installer scripts, instructions, source/license references and per-file checksums.

## Linux verification
Portable DSP/FLIP/WAV tests pass with GCC C++20, -Wall -Wextra -Wpedantic -Werror and AddressSanitizer/UndefinedBehaviorSanitizer. Leak detection is disabled because the sandbox uses ptrace. Plugin JUCE code itself was not sanitizer-instrumented.

## Still unverified or limited
FL Studio manual compatibility, Windows 10/11 desktop/device/high-DPI behavior, standalone UI launch and UAC launcher interaction. Exhaustive host automation, seeks, offline DAW bounces and real-time performance certification remain manual/Phase 5 work. Existing content/audition try-locks can skip or silence one contended block. Sustained-note chasing is not implemented.

WEE FLIP is a rendered-audio transform and temporary A/B audition, not an automatically regenerated live time-stretch instrument. Audition caches are not stored in projects; notes and all settings are stored. Re-render after reopening, edits or tempo/rate changes. Half-time slows the first half to the original duration and lowers pitch; transpose is granular. Seamless mode uses warm-up plus short edge fades, which can create a tiny boundary dip. User-selected tail cutoffs may truncate remaining decay. Wet stems need not sum exactly through nonlinear/noise effects. The renderer rejects over 16 million frames and other unsupported extremes explicitly. See PHASE-3.md.

Future phases: advanced composition, preset files/browser, piano-roll editing, persistent history and production polish/performance. Earlier verified Phase 2 build remains at https://github.com/treezysosa/WEE/actions/runs/38826086035.
