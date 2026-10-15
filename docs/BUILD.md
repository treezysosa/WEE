# Windows build
Install Visual Studio 2022 with Desktop development with C++, Windows SDK, CMake 3.22+, and Git. Run `./build-windows.ps1` from PowerShell. Uses VS 17 2022, x64, Release and CTest. Supply `-JuceSourceDir C:\dependencies\JUCE` for an offline JUCE 8.0.6 checkout. JUCE is pinned to release 8.0.6; do not substitute an arbitrary checkout.

Expected output after a successful Windows build: `build/WEE_artefacts/Release/VST3/WEE.vst3`. Standalone: `build/WEE_artefacts/Release/Standalone/WEE.exe`.

Close FL Studio, open administrator PowerShell, run `./install-vst3.ps1`. Existing installations require explicit `-Replace` and receive a timestamped backup. Installation failures are errors, never silent success.

Portable core checks without JUCE: `cmake -S . -B build-core -DWEE_BUILD_PLUGIN=OFF`, `cmake --build build-core`, `ctest --test-dir build-core --output-on-failure`.

Current verification: portable core tested on Linux. JUCE plugin build, Windows MSVC build, standalone launch and FL Studio tests require further verification. No VST3 binary is included.
