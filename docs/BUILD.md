# Windows build
Install Visual Studio 2022 Desktop development with C++, Windows SDK, CMake 3.22+ and Git. Run `./build-windows.ps1` from PowerShell. The script configures Visual Studio 17 2022 / x64, builds Release and runs CTest. Use `-JuceSourceDir C:\dependencies\JUCE` for an existing JUCE 8.0.6 checkout; default builds fetch that release.

Output: `build/WEE_artefacts/Release/VST3/WEE.vst3` and `build/WEE_artefacts/Release/Standalone/WEE.exe`. Run administrator PowerShell `./install-vst3.ps1` to install from a source build. Close FL Studio first. Existing installations require `-Replace` and receive a timestamped backup. The downloaded package instead includes Install-WEE.cmd / Install-WEE.ps1 beside its ready-built WEE.vst3 folder.

Portable tests without JUCE: `cmake -S . -B build-core -DWEE_BUILD_PLUGIN=OFF`, `cmake --build build-core`, `ctest --test-dir build-core --output-on-failure`.

Phase 3 verification: Windows Visual Studio 2022 x64 Release VST3 and standalone builds passed; all four CTest suites and installer/hash/backup checks passed. The instrument, state, WAV/FLIP rendering and worker tests run on the Windows runner. See VALIDATION.md for source commit, artifact and exact coverage. Standalone device/UI launch, UAC interaction, visual high-DPI checks and FL Studio manual compatibility remain unverified.
