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

Implemented in this Phase 1 build: seeded 8/16-bar melody generation, three starter styles, synthesized piano, host sync, internal preview, MIDI file save/drag, generation undo/redo, automation and project state.
Not yet implemented: WAV export, WEE FLIP, layers, additional instruments, preset files and piano-roll editing. FL Studio manual compatibility testing is still required.

To export: SAVE MIDI then drag the saved .mid into FL Studio Piano Roll, or use Piano Roll's MIDI import. DRAG MIDI attempts native file dragging; saving to disk is the reliable fallback.

The GitHub Actions build runs automated composition/MIDI, audio and state tests. It does not run FL Studio. JUCE is licensed separately; see JUCE-LICENSE.md.
