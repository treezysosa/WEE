# Standalone startup update 0.5.1 — passed 2026-10-16

- Build: https://github.com/treezysosa/WEE/actions/runs/38841213535
- Compiled source: f52652860d3e332ba1d19c7dabbf1792cac72d85; later changes are documentation only.
- MSVC x64 Release VST3 and standalone compiled with the static C++ runtime. dumpbin import checks passed for both binaries: no external MSVC runtime imports matched the check.
- All six CTest suites passed in 14.83 seconds; installer recovery, packaged/developer install and checksum checks passed.
- Real standalone launched on the Windows runner under both PowerShell 7 and Windows PowerShell. Both runs logged a visible WEE window, a responsive Windows-message check, survival of a three-second observation, and normal zero-exit-code shutdown.
- Windows loaded the VST3 module and located GetPluginFactory in both sessions. This is not a full plugin validator or FL Studio test.
- Download: https://github.com/treezysosa/WEE/actions/runs/38841213535/artifacts/12021278399
- ZIP SHA-256: 68f41dc81dbd343c1d01cbe8a740b74bf6f45ef8ef01b88adc7b7a5da21e01ed
- Expires 2026-11-15. Package includes Start-WEE.cmd/.ps1 and STARTUP-HELP.txt, plus the plugin, standalone and installer/verifier files.

The user reported that the earlier WEE.exe showed no window even after the suggested runtime installation. That user-specific failure has not been reproduced or diagnosed conclusively. Static linking removes a deployment dependency; successful runner launch does not prove the user's underlying problem is repaired. The new launcher records process/window/exit-code results and available local Windows crash events to support the next diagnostic step. It requires no administrator access and uploads nothing. Logs may contain account paths; share the relevant FAIL line first.

Physical audio devices, user security policies, Windows 10/11 desktop/high-DPI behavior, and FL Studio scan/load/playback still require manual verification. See WINDOWS-STARTUP.md. Existing plugin IDs and version 4 project-state format are unchanged.

# Phase 5 validation — passed 2026-10-16

- Windows build: https://github.com/treezysosa/WEE/actions/runs/38834778684
- Compiled source: e2e6b8284eec8432d725b805f4b768cc6e7cf355 (later changes are documentation only).
- Windows Server 2022 GitHub runner, MSVC 2022 x64 Release, C++20, JUCE 8.0.6; plugin version 0.5.0. Plugin identity/parameter order and version 4 state format retained.
- VST3 and standalone compiled. All 6 CTest suites passed: composition_midi, layer_harmony_midi, dsp_flip_wav, schedule_reference, installer_recovery, processor_audio_state.
- Scheduler checks independently compare the prior full scan against event timelines across 8/16 bars, 8/44.1/48/96 kHz, five tempos, four block sizes and ten host positions including negatives/wraps. Exact-offset cancellation and same-pitch boundary regressions pass. Dense 2,048-note timeline: 100,000 two-stream lookups measured 4.4307 ms (26,040 emitted events; descriptive timing, no timing gate).
- Gesture tests simulate mouse down/up and keys on the real piano-roll component: click selection survives release, Delete removes the note, Escape cancels a draft, stale selection preserves replacement content, cached display follows revisions, and editing one layer leaves unrelated voice revisions unchanged. Existing editor lifecycle, state/history/preset, musical lock, MIDI, actual synthesis/effects and WAV/FLIP/stem tests pass.
- Meters record actual output; local diagnostic report fields tested. 18 callback profiles cover eight enabled layers at 44.1/48/96 kHz and 64/256/1024 samples, effects off/on. Audio is finite and audible. Short profiles use 180 callbacks, excluding the first 20; this is not worst-case/full-loop/128-voice/device/DAW performance certification.

Representative callback measurements in microseconds on this runner:

| Rate | Block | Effects | Median | p99 | Buffer budget |
|---|---:|---|---:|---:|---:|
| 48 kHz | 64 | On | 62.5 | 91.0 | 1333.3 |
| 48 kHz | 256 | Off | 248.4 | 293.9 | 5333.3 |
| 48 kHz | 256 | On | 296.5 | 363.1 | 5333.3 |
| 96 kHz | 64 | On | 61.4 | 78.3 | 666.7 |
| 96 kHz | 1024 | On | 1166.9 | 1345.9 | 10666.7 |

- Windows PowerShell installer tests pass in both 64-bit CTest and a separate 32-bit process. They cover manifest integrity/traversal rejection, fresh install, unapproved replacement refusal, staging-copy failure, failed publication with rollback, successful replacement/backup bytes and staging cleanup.
- Actual VST3 installed at C:\Program Files\Common Files\VST3\WEE.vst3. Packaged installer replacement and developer install-vst3.ps1 replacement both passed; retained backups and installed file checksums verified by Verify-WEE.ps1.
- Download: https://github.com/treezysosa/WEE/actions/runs/38834778684/artifacts/12018846261
- ZIP SHA-256: 5bf09b49e51c9c6f6796d4e1529380dbe04362c9f2c161ded5de2a194a9e55b8
- Expires 2026-11-15. Includes VST3, standalone, installer/core/verifier, instructions, source/license references and checksums.
- Local GCC: portable composition/layer/DSP and scheduler suites pass; scheduler passes strict warnings and AddressSanitizer/UndefinedBehaviorSanitizer with sandbox leak detection disabled. Processor/JUCE code was not sanitizer-instrumented.

FL Studio interactive scan/playback/import/project reopening, Windows 10/11 physical audio-device/driver/high-DPI behavior, actual mouse interaction/hover rendering, native file drags/dialogs, UAC launcher prompts, loaded-plugin file locking and desktop backup recovery remain unverified. Simulated gesture tests and runner filesystem installation do not complete those manual gates. Follow FL-STUDIO-SETUP.md and save a local diagnostic report for failures.

Earlier installer runs failed on unavailable legacy Get-FileHash discovery and a leftover argument during its replacement; both were corrected before the successful build above. The downloadable artifact is from the final passing run. See PHASE-5.md for remaining production/performance/editor work and synchronization limits.

# Phase 4 validation — passed 2026-10-16

- Windows build: https://github.com/treezysosa/WEE/actions/runs/38831830185
- Compiled source: 5b4cb75bcb0257ccdfc7b4dbb751f789313aa29b
- Windows Server 2022 runner, MSVC 2022 x64 Release, C++20, JUCE 8.0.6; plugin version 0.4.0.
- VST3 and standalone compiled; all 4 CTest suites passed in 2.40 seconds.
- New processor checks: valid edit commit, grid/pitch/end bounds, exact undo/redo, history surviving project reload and editor closure, rejection of invalid/locked edits, exact preset loading, undoable preset replacement, malformed preset preservation, eight-transaction cap, deterministic moods and mood/note state restoration.
- Prior composition, 432 layer harmony contexts, independent MIDI/WAV parsing, actual instruments, master DSP, FLIP, stems, renderer, host/meter, state migration and editor regression suites pass.
- Installer installed the full bundle at C:\Program Files\Common Files\VST3\WEE.vst3; installed binary hash matched the build; explicit replacement created a timestamped backup.
- Download: https://github.com/treezysosa/WEE/actions/runs/38831830185/artifacts/12018265517
- ZIP SHA-256: e73e7a934c341b9e7ed45163de0654debba7766ae771da49771084e63fa6787d
- Artifact expires 2026-11-15. Includes VST3, standalone, installer, Phase 4 instructions, source/license references and per-file checksums.
- Linux GCC: portable core/layer/DSP suites pass. Core builds with strict warnings; layer generation and a grid/pitch/end edit-bound sweep pass AddressSanitizer/UndefinedBehaviorSanitizer (sandbox leak detection disabled). JUCE processor code was not sanitizer-instrumented.

FL Studio manual playback/import/project reopening, real Windows 10/11 desktop/audio-device/high-DPI interaction, interactive piano-roll gestures, file dialogs, drag/drop and UAC launcher interaction remain unverified. CI editor construction/resizing and data-layer editing tests do not substitute for these manual checks. Phase 4 remains incremental: zoom/multi-selection, preset search/tags and richer musical phrasing remain future work. See PHASE-4.md and FL-STUDIO-SETUP.md.

The initial Phase 4 test compilation failed on an entry-point declaration; it was corrected before the successful build above. Earlier canceled/failed runs are not the downloadable verified build.

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

Further work: advanced composition, richer preset browsing/editing and production polish/performance. Earlier verified Phase 2 build remains at https://github.com/treezysosa/WEE/actions/runs/38826086035.
