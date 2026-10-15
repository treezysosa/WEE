# Phase 2

Eight layers share one explicit key, scale, 4/4 loop length and progression. Lead, counterline, voice-led chords, piano arpeggios, guitar picking, sustained strings/pad and sparse vocal texture have separate generated note sequences. All layers are generated even when disabled, so ON immediately makes their content available. Changing a selector sets the next generation request; existing music is preserved until Generate succeeds.

Eight additive/FM timbres use harmonic weights, envelopes and detuning; no samples or licenses for recordings are required. Guitar and vocal sounds are synthesis approximations. Instruments switch at the next audio block, releasing existing voices. MIDI input is routed to the Lead instrument; generated layers play from preview or the host transport.

Each layer exposes enabled, mute, solo, volume, pan, instrument and note lock parameters to the host. Solo affects enabled layers; mute wins over solo. A disabled solo layer does not suppress other layers. Pan is equal-power. Gains and panning are smoothed over 20 ms.

Lock preserves notes through Generate, while mixer/sound controls remain editable. Locked layers block global key/scale/style/bar changes. REGEN is disabled on locked rows and generates only one unlocked layer using the saved arrangement context. Global Generate uses fresh seeds for unlocked layers. Undo/redo of generation restores notes, parameters and the next seed; history itself lasts for the current editor session.

SAVE ALL MIDI and DRAG MIDI use format 1 with a tempo track and one named track/channel per audible layer. Layer MIDI buttons use format 0 regardless of mute/enable state. Both have exact nominal loop-end metadata. Phase 3 will add WAV/stem rendering; no WAV control is advertised yet.

State version 2 stores the entire arrangement, per-layer seeds and APVTS mixer state. Version 1 is migrated without regenerating the saved lead. Extra layers receive generated content but remain disabled. Existing style indices 0/1/2 remain Pop/Dark Pop/Indie Pop; because the choice list expanded, review any Phase 1 automation lanes targeting Style after upgrading (normalized host automation may map differently). Saved note content itself is preserved.

Audio buffers and MIDI capacity are prepared outside processing; oversized host blocks are split into preallocated chunks. Audio tries the content lock and releases voices instead of waiting if an edit contends. A contended block can miss a new onset; immutable publication is future work. Host seeks clear old voices but do not chase notes already held before the seek. Normal note scheduling is sample-accurate within each chunk. WAV export, effects and WEE FLIP remain Phase 3.

Manual FL Studio checks: enable all layers, isolate solos and mute, pan hard left/right, switch each instrument, lock a melody and Generate, confirm incompatible context changes preserve music with a warning, export/import each layer and the multitrack file, save/reopen a layered project, then reopen a Phase 1 project. Verify automation, repeated generation, multiple instances, resizing/high DPI, transport seeks, sample-rate changes and offline rendering.
