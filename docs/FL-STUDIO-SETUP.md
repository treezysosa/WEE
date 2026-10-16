# Manual FL Studio validation — NOT YET RUN
1. Build on Windows and install WEE.vst3 to `C:\Program Files\Common Files\VST3`.
2. Open FL Studio 64-bit, Options > Manage plugins, scan, locate WEE as a generator, add to Channel Rack.
3. Select Dark Pop, A, Minor, 8 bars; set preview and host BPM to 120; Generate.
4. Use PLAY / STOP with host stopped, then disable preview and start host transport with Host sync enabled. Verify pitch, timing, stop/start and BPM changes.
5. SAVE MIDI to disk. Drag the .mid file into Piano Roll or use Piano Roll File > Import MIDI file. DRAG MIDI attempts native file dragging; use the saved file when unsupported by the host.
6. Save the FL project, close/reopen, verify exact notes, seed, settings and audio. Test multiple instances, repeated generation, resizing/high DPI, transport seeks, sample-rate changes, offline audio rendering, plugin removal and shutdown. Test non-4/4 warning.
7. Phase 3: export exact 8/16-bar WAVs at all three bit depths, import into Playlist, verify loop seams, durations and tail/stem options. Use FX / FLIP / WAV; see PHASE-3.md for exact export semantics and remaining manual checks.

Automated now: portable generation determinism/bounds and independent MIDI event/duration parser; JUCE MIDI parsing; MIDI-input piano audio; generated-note and gain state round-trip into the same and fresh instances; stopped-host preview audio; preview stop; host tempo and unsupported-meter detection. Windows VST3 and standalone compilation plus installer/backup checks passed in CI. Still needed: exhaustive scheduling/seek and offline rendering tests, standalone UI/UAC checks, plugin validator and all manual FL Studio checks. Passing automated tests does not establish FL Studio compatibility.


## Phase 2 checks
See PHASE-2.md. For separate Piano Rolls, use each row's MIDI button (format 0). For a full layered arrangement, use SAVE ALL MIDI and FL Studio's File > Import > MIDI file workflow to choose/import the named tracks (format 1). Host import behavior still requires manual verification. Check any Phase 1 Style automation lane after upgrading because the choice list expanded; saved generated notes migrate unchanged.

## Phase 4 manual checks

Select a layer in CREATE, open EDIT / LIBRARY, then add, move, resize, adjust velocity and delete notes. Verify locks, Escape cancellation and all grid choices. Undo and redo each gesture. Close/reopen the plugin window and verify history remains. Save/reopen an FL Studio project and verify exact edited notes, settings and undo/redo. Save a .wee preset, generate different content, load the preset and undo the load. Try every factory preset, all three moods, preset overwrite confirmation and an invalid file. Drag exported MIDI into Piano Roll, then close the editor and confirm the file is still usable. These checks require FL Studio and are not implied by automated CI passing.

## Phase 5 manual release gate (not yet executed)

Record Windows/FL Studio versions, device/driver, sample rate and buffer size. Run Verify-WEE.cmd after installation. Check glossy hover rendering and tooltips at 100/150/200 percent display scaling; test compact/large windows. Confirm L/R meter movement, decay and clip indication agree with audio output. Click a note then Delete; verify stale selections and Escape preserve replacement music. Test eight-layer CPU at 44.1/48/96 kHz and 64/256/1024 samples with effects off/on, multiple instances, host seeks/loop wraps, transport, tempo/rate changes, offline DAW export, editor closure, plugin removal and application shutdown. Check MIDI/WAV drag and import, presets and exact edited-state/history recall. Exercise UAC installation, declined replacement, backup restoration and a plugin held open by a host. SAVE REPORT produces a local diagnostic file for a reproducible failure. CI does not execute this desktop/DAW gate.
