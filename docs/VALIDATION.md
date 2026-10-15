# Validation in this environment
Linux x86-64, GCC C++20. Portable CoreTests compiled with -Wall -Wextra -Wpedantic -Werror and passed. AddressSanitizer and UndefinedBehaviorSanitizer passed with leak detection disabled because this runtime uses ptrace, which LeakSanitizer cannot support.

CMake, JUCE, MSVC and Windows are unavailable. Dependency download retry failed: configured proxy could not be reached. JUCE processor/editor compilation, standalone launch, VST3 build, state round-trip and all FL Studio tests remain unverified. No WEE.vst3 was generated. This source is an initial Phase 1 implementation requiring a Windows build/review, not a release-ready complete plugin.
