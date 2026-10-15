# Architecture and implementation plan
1. Phase 1: composition/SMF core; JUCE piano/processor; white/cyan editor; state, automation, host sync; Windows build/install scripts. Verify MSVC and FL Studio before release.
2. Phase 2: eight shared-harmony layers, new instruments, styles, mixer and locks.
3. Phase 3: smoothed effects, worker-thread WAV rendering, exact-duration loops/tail files, WEE FLIP and stems.
4. Phase 4: advanced composition, preset library, editing, persistent history and richer drag UX.
5. Phase 5: performance profiling, visual polish, installer and manual compatibility matrix.

Music/Composition.h is a deterministic portable generator using an explicit LCG, scale degrees, a repeating progression, chord-tone phrase anchors, neighboring motion, phrase-end resolution, rests and velocity/timing variation. It is a starter musical engine, not the full requested composition system.
Export/Midi.h writes format-0 SMF at 960 PPQ with tempo, 4/4 and exact loop-end metadata. CoreTests independently parses event timing, lengths and note counts.
Instruments/Piano.h is an additive decaying harmonic synth, not a sampled acoustic piano. MIDI and generated events share 24 voices.
The processor uses APVTS for automation. Generated note data is serialized explicitly, so reopening does not regenerate the melody. Serialization has a version check and validates note bounds. Generated content uses a spin lock: audio only tries it and skips scheduling for a contended block, never waits. UI edits allocate outside that critical section; snapshot/state copies can hold the lock. A future immutable fixed-buffer swap should remove skipped events under heavy concurrent editing. JUCE MIDI buffering may allocate at high event densities; hard realtime certification is pending. Host seeks do not chase sustained notes; a future scheduler should track discontinuities and held notes. Undo restores generated notes, while selector values remain the current generation settings.
4/4 is required. Unsupported host signatures stop generated playback with a visible message. BPM is read every block. Host PPQ drives sync, preview otherwise advances locally. Host positions missing PPQ fall back to local progression.
