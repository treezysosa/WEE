# Manual FL Studio validation — NOT YET RUN
1. Build on Windows and install WEE.vst3 to `C:\Program Files\Common Files\VST3`.
2. Open FL Studio 64-bit, Options > Manage plugins, scan, locate WEE as a generator, add to Channel Rack.
3. Select Dark Pop, A, Minor, 8 bars; set preview and host BPM to 120; Generate.
4. Use PLAY / STOP with host stopped, then disable preview and start host transport with Host sync enabled. Verify pitch, timing, stop/start and BPM changes.
5. SAVE MIDI to disk. Drag the .mid file into Piano Roll or use Piano Roll File > Import MIDI file. DRAG MIDI attempts native file dragging; use the saved file when unsupported by the host.
6. Save the FL project, close/reopen, verify exact notes, seed, settings and audio. Test multiple instances, repeated generation, resizing/high DPI, transport seeks, sample-rate changes, offline audio rendering, plugin removal and shutdown. Test non-4/4 warning.
7. After Phase 3: export exact 8/16-bar WAVs at all three bit depths, import into Playlist, verify loop seams, durations and tail/stem options. WAV export is currently unavailable.

Automated now: portable generation determinism/bounds and independent MIDI event/duration parser. Still needed: JUCE state round-trip tests, host scheduling tests, audio rendering tests, plugin validator and all manual host checks. Passing core tests does not establish FL Studio compatibility.
