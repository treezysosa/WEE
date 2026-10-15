WEE — Windows x64 development build

1. Extract the ENTIRE WEE-Windows-x64 ZIP to a folder.
2. Close FL Studio.
3. Double-click Install-WEE.cmd and approve the Windows administrator prompt.
4. Open FL Studio > Options > Manage plugins > Find installed plugins.
5. Add WEE as an instrument/generator in the Channel Rack.

Manual installation: copy the entire WEE.vst3 folder into
C:\Program Files\Common Files\VST3\
Do not copy just the internal binary.

Double-click WEE.exe for standalone testing; configure your audio device in its settings.
If Windows reports a missing Visual C++ runtime, install Microsoft's x64 Visual C++ 2015-2022 Redistributable:
https://aka.ms/vs/17/release/vc_redist.x64.exe
This development build is unsigned. SmartScreen may identify an unknown publisher.

Phase 2 build: eight shared-harmony layers, eight synthesized instruments, nine styles, enable/mute/solo, volume/pan, layer locking and isolated regeneration, per-layer and multitrack MIDI export, complete generation undo/redo, host sync, automation and versioned project state. Only Lead is enabled initially; turn other layers ON. Click a layer name to inspect it.
Not yet implemented: WAV/stem export, effects, WEE FLIP, preset files and piano-roll editing. FL Studio manual compatibility testing is still required.

To export: SAVE ALL MIDI then drag the saved .mid into FL Studio Piano Roll, or use Piano Roll's MIDI import. DRAG MIDI attempts native file dragging; saving to disk is the reliable fallback. Full export follows enable/mute/solo; each layer MIDI button exports its notes independently. Global generation refuses key/scale/style/length changes while notes are locked, preserving your existing music.

The GitHub Actions build runs automated composition/MIDI, audio and state tests. It does not run FL Studio. JUCE is licensed separately; see JUCE-LICENSE.md.
