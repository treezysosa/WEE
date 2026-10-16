# WEE — Phase 3 development build (0.3.0)
Windows-first JUCE VST3 instrument and standalone app. Eight generated musical layers, eight original synthesized instruments, nine styles, MIDI export, mixer/locks and exact generated-note state remain available.

Phase 3 adds real master-bus reverb, tempo-synced delay, chorus, saturation, low/high-pass filters, stereo width, wow/flutter and gated vinyl noise. Controls smooth over 20 ms; enable Live effects to hear them.

The FX / FLIP / WAV page renders real audio on a background worker. WEE FLIP offers half-time, reverse, duration-preserving granular pitch transpose, tape warmth, wow/flutter, filtering, reverb and vintage character. RENDER A/B builds a frozen snapshot; ORIGINAL / FLIPPED starts audition, LIVE returns to the synthesizers. Mixer/music/effect edits invalidate the cached snapshot. Master gain remains live. A/B audio is temporary; project state stores generated notes and all effect/FLIP settings, not large rendered buffers. Re-render A/B after reopening or changing tempo/sample rate.

SAVE WAV exports the audible mix; EXPORT STEMS exports enabled audible layers separately; each row's WAV button renders that layer regardless of mute/enable. Export supports stereo 16-bit PCM, 24-bit PCM and actual 32-bit IEEE float WAV at the host sample rate (48 kHz fallback before the host supplies a rate). Main files always contain round(bars * 4 * 60 / BPM * sampleRate) frames. Optional tails go into separate files. Seamless mode uses instrument/effect warm-up plus 2 ms boundary fades without extending duration. Each export gets a fresh folder; DRAG WAV and OPEN EXPORT FOLDER provide FL Studio import paths.

[Download WEE Phase 3 for Windows](https://github.com/treezysosa/WEE/actions/runs/38829866854/artifacts/12017081048) — sign in to GitHub, extract the entire ZIP, close FL Studio and run Install-WEE.cmd. Artifact retained until November 15, 2026.

Windows x64 Release VST3/standalone compilation, all four CTest suites and installer/replacement checks passed on 2026-10-16. Portable DSP/transform/WAV tests, sanitizers and independent Python PCM imports also pass. See docs/PHASE-3.md for exact half-time semantics, render limits and known limitations. FL Studio manual compatibility remains unverified.

Future work: advanced composition, preset browser/files, editable piano roll, persistent history, high-quality time stretching and further visual/performance polish. No account, subscription, network or commercial sample library is needed for generation. JUCE licensing applies separately; an existing JUCE 8.0.6 checkout enables offline builds.
