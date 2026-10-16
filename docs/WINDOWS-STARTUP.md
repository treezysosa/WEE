# WEE opens no standalone window

Version 0.5.1 statically links the Microsoft C++ runtime. This removes one deployment dependency; the cause of an individual user's silent startup is not established just by making that change.

1. Download the 0.5.1 package and extract the entire ZIP into a new folder.
2. Run Start-WEE.cmd normally. Administrator access is unnecessary for standalone use.
3. The console reports whether WEE exits before opening a window, stays running without showing one, or opens a responsive window. On failure, copy its FAIL line and exit code. A local log is saved under the temporary WEE-startup folder. It may include Windows account paths and crash event details; share only the relevant error initially. Nothing is uploaded.
4. If WEE opens, use Options > Audio/MIDI Settings to choose your output device, then PLAY PREVIEW. The launch check alone does not test physical audio devices.

For FL Studio, run Install-WEE.cmd with FL Studio closed, then scan for WEE in FL Studio's Plugin Manager. The plugin bundle installed at C:\Program Files\Common Files\VST3\WEE.vst3 is separate from the standalone WEE.exe. A successful standalone launch or native module load does not establish FL Studio compatibility. Report the FL Studio version and its exact scan/load message if that still fails.

CI now actually launches the standalone executable, requires a visible window responding to Windows messages, observes it briefly, and requests a normal close with a zero exit code. It also asks Windows to load the VST3 module and checks the GetPluginFactory export. dumpbin verifies that the packaged binaries do not import the external MSVC runtime DLLs. Existing processor/DSP/state/MIDI/WAV/installer tests remain required. These runner checks do not reproduce a user's security policy, audio driver, desktop or DAW configuration.
