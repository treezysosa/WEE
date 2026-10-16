# Phase 3 — effects, audio exports and WEE FLIP

## Live effects
Enable Live effects on FX / FLIP / WAV. All nine controls process real stereo audio: Schroeder reverb, BPM-synced quarter-note delay (feedback 0.42), modulated chorus, normalized tanh saturation, one-pole low/high-pass filters, equal-power layer pan plus master mid/side width, modulated tape delay for wow/flutter and source-gated synthetic vinyl hiss/crackle. Parameters ramp over 20 ms. Processing allocates its rings during prepare, not inside the effect processing loop. No recordings or external samples are included.

## WEE FLIP
Controls process a rendered snapshot, not live MIDI. Half-time plays the FIRST HALF of the source at half speed to fill the same selected 8/16-bar duration, lowering pitch one octave before Transpose. Reverse reverses audio samples. Transpose uses two overlapping grains to change pitch while retaining duration; this is a granular effect, not a high-quality formant-preserving stretch engine. Tape warmth, wow/flutter, filter, reverb and Vintage character run through real DSP on the transformed audio.

RENDER A/B runs in the background. ORIGINAL and FLIPPED audition the same frozen arrangement/mix snapshot, with a 20 ms comparison crossfade. LIVE returns to current synthesis. Master volume stays live. Music/mixer/effect edits invalidate audition and return to LIVE. Host tempo changes require re-rendering (cached audio is invalidated to avoid unintended repitching). Reopening projects restores the notes/settings; audition audio must be rendered again and is not serialized. Return to LIVE for ordinary DAW instrument rendering; use Apply FLIP to WAV for authoritative transformed exports.

## WAV export
Generation, MIDI export and WAV/A/B rendering refuse detected unsupported host meters with an explicit 4/4-required message. Existing music is preserved.
Choose 16-bit PCM, 24-bit PCM or 32-bit IEEE float. Float files use format tag 3 and a fact chunk; they are not integer PCM mislabeled float. The prepared host sample rate is used, with a documented 48 kHz fallback before preparation. WAVs have two channels. Full exports follow enable/mute/solo; individual layer WAV buttons override that layer's mute/enable for export. Wet stems apply the same master chain independently, so nonlinear/noise processing means their sum need not equal the wet full mix.

Workers clone the exact saved processor state and render its actual instruments and effect engine. A frame-rounded host clock supplies tempo/PPQ. Two warm-up cycles precede capture so release/reverb/delay energy can wrap into the beginning. The main file has exactly round(bars * 4 * 60 / BPM * rate) frames. Seamless mode adds 2 ms raised-cosine edge fades to zero for click-free boundaries; this can create a tiny boundary dip. Turning it off retains exact duration without that conditioning.

When seamless mode is active, tails also receive short edge fades to join the zeroed loop boundary. Separate tail exports create an additional -tail.wav of the chosen 1–30 seconds, never extra frames on the loop. Tails can still contain sound at the chosen cutoff; choose a longer duration for slow delays. Original tails include instrument releases and live FX; flipped tails contain the transform-chain decay (source releases already wrap within the sampled loop). Peak protection only attenuates if peak would exceed 0.98; it uses the same gain for a loop and its tail. Disable it to retain float headroom; integer output then clips at PCM bounds.

Exports create a unique WEE timestamp folder. Temporary partial files are renamed after successful writing; cancellation removes the incomplete file and retains completed ones. Render-info records BPM, bars, rate and format. DRAG WAV attempts a native file drag; OPEN EXPORT FOLDER is the reliable fallback for FL Studio's Playlist. The installer remains unchanged and backs up existing WEE installations.

Render limit: 16 million stereo frames per loop, rates 8–384 kHz and tempos 20–999 BPM. Extreme combinations fail explicitly rather than allocating unbounded memory. Rendering is sequential across stems. Closing the editor cancels and joins its worker on the UI thread; no file/render work runs on the audio thread.

## State and limitations
State version 3 stores new parameters; versions 1/2 migrate with live FX bypassed. MIDI and note locks are unchanged. A same-pitch loop-boundary scheduling bug was fixed by inserting note-offs before note-ons at the same sample.

Cached audition uses an audio-thread try-lock; a contended publication can silence one block. Existing note-content try-lock and missing sustained-note chasing remain Phase 2 limitations. External MIDI shares the Lead engine. Real-time certification, exhaustive host/offline tests, standalone device/UI launch and FL Studio manual compatibility are still pending.

## Manual FL Studio checks
Install, scan as generator, enable each effect and automate it during transport. Test stop/start, seeks, tempo/sample-rate changes and multiple instances. Render A/B and compare original/flipped; adjust each FLIP control then re-render. Export 8/16-bar files at integer/fractional tempos and all depths; import into Playlist and check frame duration, stereo, seams, tails and stems. Try DRAG WAV and folder fallback. Save/reopen projects and verify generated MIDI and effect settings; re-render A/B. Test cancellation and plugin/editor removal during rendering.
