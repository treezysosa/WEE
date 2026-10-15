# WEE — Phase 2 development build
Windows-first JUCE VST3 instrument and standalone app. Original white/cyan design with no Nintendo assets, recordings or commercial samples.

Phase 2 adds eight shared-harmony layers: Lead Melody, Counter Melody, Chords, Piano, Guitar, Strings, Pad and Vocal Texture. Each has enable/mute/solo, smoothed volume and equal-power pan, instrument selection, note locking, isolated regeneration and MIDI export. Only Lead is enabled initially; turn other layers ON to hear them. Click a layer name to inspect its piano roll.

Eight original synthesized instruments: piano, electric piano, acoustic-style guitar, clean electric guitar, strings, analog pad, vocal synth and soul keys. These are synthetic approximations, not sampled acoustic instruments. Nine generation styles: Pop, Dark Pop, Indie Pop, Guitar, Piano, Strings, Pads, Vocal and Soul.

Generation is deterministic, seed-based, scale-aware and uses a shared progression. Lock preserves a layer's exact notes. Global generation preserves locked layers and refuses harmonic-context changes until all layers are unlocked. Per-layer regeneration uses the existing arrangement's context. Layer controls remain editable while notes are locked. Generation undo/redo restores the complete arrangement, mixer and seed.

SAVE ALL MIDI / DRAG MIDI export enabled audible layers as format-1 MIDI with named tracks and separate channels; each layer's MIDI button exports it independently even if muted. Project state version 2 stores every layer's notes/settings and migrates Phase 1 project states with the original melody preserved and added layers disabled.

Windows build verification for this revision is pending. The previously verified Phase 1 build remains available in GitHub Actions. See docs/VALIDATION.md. FL Studio compatibility is unverified.

Still future work: WAV/stem export, effects, WEE FLIP, piano-roll editing, preset browser, persistent history and advanced visual polish. No fake WAV/effect controls are shown. Initial JUCE download needs network; an existing JUCE 8.0.6 checkout supports offline builds. Generation needs no network, subscription or account. JUCE licensing is separate from the original source license.
