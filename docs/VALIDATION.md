# Validation

## Windows remote build — passed 2026-10-15
- Build: https://github.com/treezysosa/WEE/actions/runs/38822919717
- Source commit: 668f6f89dd48b83b34cf79451050579cc24e5d86
- Toolchain: Windows Server 2022 GitHub runner, Visual Studio 2022, x64 Release, JUCE 8.0.6, C++20.
- Built WEE.vst3, WEE.exe standalone, portable core tests and JUCE processor tests.
- CTest: 2/2 passed (composition_midi and processor_audio_state).
- Processor checks: MIDI-input piano audio, independent JUCE SMF parser, generated-note/seed and gain state recall, fresh-instance recall, stopped-host preview, preview stop, tempo following and unsupported-meter warning.
- Packaged installer installed the VST3 to C:\Program Files\Common Files\VST3\WEE.vst3. Hash matched the build. Replacement created a timestamped backup.
- Artifact: https://github.com/treezysosa/WEE/actions/runs/38822919717/artifacts/12013663981
- ZIP SHA-256: e5a0981c139c884b6f0ac4589d2f1534a88c5e6c6ffedf505097e371139f0c31
- Download includes VST3 bundle, standalone executable, installer scripts, source links, licenses and SHA256SUMS.txt. GitHub retains it for 30 days, until 2026-11-14.

## Linux core checks — passed
GCC C++20, -Wall -Wextra -Wpedantic -Werror. AddressSanitizer and UndefinedBehaviorSanitizer passed with leak detection disabled because this runtime uses ptrace, which LeakSanitizer cannot support.

## Still unverified
FL Studio manual compatibility, standalone UI launch/device configuration, UAC interaction from Install-WEE.cmd, exhaustive transport-seek/scheduling behavior, and Windows 10/11 desktop manual tests. This is a Phase 1 development build; WAV export, effects, WEE FLIP, layers and additional instruments remain future work.
