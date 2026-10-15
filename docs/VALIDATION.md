# Phase 2 validation — passed 2026-10-15

- Remote build: https://github.com/treezysosa/WEE/actions/runs/38826086035
- Compiled source: af5d519c9d812639105beba8c8b823f618c6bc28
- Toolchain: GitHub Windows Server 2022 runner, Visual Studio 2022 x64 Release, C++20, JUCE 8.0.6. Plugin version 0.2.0.
- Built stereo VST3 instrument and standalone executable.
- CTest: 3/3 passed — composition_midi, layer_harmony_midi, processor_audio_state.
- Portable tests: 432 key/scale/style/length contexts, all eight layers, seeded determinism, scale membership, bounds and independent format-1 MIDI parser.
- JUCE tests: MIDI-input piano; independent SMF import; exact eight-layer note/seed and parameter state recall; Phase 1 state migration; locked-note preservation; blocked incompatible generation; isolated regeneration; solo/mute masks; eight distinct audible synthetic timbres at 44.1/48 kHz; hard-left pan; actual eight-layer playback; actual mute silence; oversized render blocks; invalid-state preservation; editor creation, compact/large resize and destruction.
- Fixed a decimal timing serialization loss found by the exact layer-state test. Lossless note timing now passes the same strict equality check.
- Installer installed WEE into C:\Program Files\Common Files\VST3\WEE.vst3. Installed-binary hash matched the build. Replacement created a timestamped backup.
- Download: https://github.com/treezysosa/WEE/actions/runs/38826086035/artifacts/12015585759
- ZIP SHA-256: 27869f7d00fdba7479c4d5134d0e9e933b861f995d3e18a67edb4e22e157e929
- Artifact expires 2026-11-14. Includes WEE.vst3 bundle, WEE.exe, installer scripts, instructions, licenses, source links and file checksums.

## Linux checks
Portable core/layer tests passed with GCC C++20, -Wall -Wextra -Wpedantic -Werror. LayerTests passed AddressSanitizer/UndefinedBehaviorSanitizer with leak detection disabled because this sandbox uses ptrace.

## Still manual or unfinished
FL Studio compatibility, Windows 10/11 desktop/device and high-DPI visual checks, standalone UI launch, UAC launcher interaction, exhaustive automation/transport/offline rendering tests. Style automation lanes from Phase 1 need review because the choice list expanded. Enabling/unmuting sustained layers mid-note waits for their next onset. The audio content try-lock can skip a new onset during a concurrent edit; audio never waits for that lock. MIDI buffer growth under pathological input and JUCE internals have not been certified hard real-time.

WAV/stem export, effects, WEE FLIP, piano-roll editing and preset files are not implemented. Phase 1's earlier verified build remains at https://github.com/treezysosa/WEE/actions/runs/38822919717.
