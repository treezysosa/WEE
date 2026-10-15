# WEE — Phase 1 development build
Windows-first JUCE VST3 instrument with standalone target. Original white/cyan interface; no Nintendo assets or commercial samples.

Implemented: deterministic 8/16-bar minor/major generation in all 12 keys, three initial rhythmic styles, polyphonic synthesized piano, MIDI input, host tempo/transport following, preview playback, volume automation, piano-roll display, generation undo/redo, MIDI save/file drag, versioned project state.

This is an early Phase 1 implementation, not the complete five-phase product. Windows compilation and FL Studio compatibility are unverified. No Windows VST3 binary has been produced in this Linux environment. See docs/BUILD.md and docs/FL-STUDIO-SETUP.md.

Not implemented: editable piano roll, mood controls, preset browser/files, additional instruments and six additional styles, layers/mixer/locks, effects, WAV/stem rendering, WEE FLIP, animated menu tiles, persistent undo history. Future menu pages are deliberately absent until functional.

Initial dependency fetch needs network access; an existing JUCE 8.0.6 checkout enables offline builds. Generation never uses a network, account or subscription. JUCE has separate licensing requirements (AGPLv3 or commercial); review before distribution.
