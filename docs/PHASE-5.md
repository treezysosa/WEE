# Phase 5 — performance, polish and installation (first increment)

Version 0.5.0 retains the existing plugin IDs, parameters, and version 4 project-state format. Old projects keep their generated notes and settings. This is a development build; FL Studio interactive compatibility remains unverified.

## Performance changes

Music/Schedule.h builds sorted note-on/off timelines when music changes, outside the audio callback. Playback uses binary search followed by the events in the current block, including loop wraps and negative host PPQ. Separate off/on streams preserve note-off priority when positions round to the same audio sample. There is no full-note scan per callback and no timeline construction there. Independent reference tests compare the event multisets against the prior scan across rates, tempos, loop lengths, block sizes and musical positions. A dense 2,048-note timeline microbenchmark reports timing without a fragile shared-runner pass/fail timing threshold.

Inactive synthesizer layers skip rendering when they have no MIDI events or live voices; smoothing still advances. Stable disabled master effects skip their sample loop, preserving exact dry output. Editing a layer updates only that layer's timeline/revision, so unrelated sustained voices are not reset. Arrangement generation and state restoration rebuild all timelines.

The piano roll caches the selected layer by content revision. Painting does not copy the whole arrangement. Idle timer repainting targets meters/status instead of the full editor. Musical changes refresh the cache and editor. Existing content try-locks remain: contention can still release voices or skip a block. Sustained-note chasing after seeks is not implemented. This is not a lock-free redesign or a certification that all host/framework paths allocate nothing.

The processor test profiles eight-layer audio at 44.1/48/96 kHz, 64/256/1024-sample buffers, and live effects off/on. It reports median, 99th-percentile and callback-budget microseconds in the test log. Audio must remain finite and non-silent; timing is observational, not a real-time guarantee. Actual FL Studio/device latency and worst-case performance still require measurement on Windows desktops.

## Interface

Buttons use original vector-drawn glossy gradients, shadows and smooth hover transitions. No Nintendo assets are used. L/R output meters show post-master peaks from -60 to 0 dBFS, decay visually, and hold an orange CLIP indicator for peaks above full scale. The meters do not change the audio or limit output. Note hover tooltips show MIDI pitch, velocity, beat and length. Click selection now survives mouse-up so Delete works; stale selections cannot delete replacement notes. Escape still cancels drafts, and locks protect edits.

SAVE REPORT writes a local text report of plugin version, sample rate, prepared block size, tempo mode, meter support, musical context and per-layer counts/seeds/audibility. It collects no user paths, samples or account data, and does not upload anything. Use it alongside the manual FL Studio checklist when reporting a problem.

## Installation and integrity

Extract the entire ZIP, close FL Studio, then run Install-WEE.cmd. The administrator installer checks SHA256SUMS.txt, refuses corruption and unsafe manifest paths, stages and hashes the entire bundle beside the destination, and only then renames an existing install to a unique backup. It publishes the staged bundle and verifies the installed files. Existing installations require an explicit YES or -Replace. If publication fails, rollback restores the previous plugin; if rollback cannot finish, the error identifies the retained backup for manual recovery. Temporary staging directories are cleaned where possible. Other audio hosts must also be closed if they have loaded the plugin.

Verify-WEE.cmd performs read-only package and installed-file checksum checks. It does not need administrator rights. Checksums detect corruption; they do not authenticate an unsigned download. Backups stay beside WEE.vst3 under C:\Program Files\Common Files\VST3. To restore one manually, close audio hosts, remove the current WEE.vst3 after retaining anything needed, and rename the selected WEE.vst3.backup-* folder to WEE.vst3 with administrator rights.

Automated isolated PowerShell tests cover fresh install, refusal of unapproved replacement, corrupted/unsafe manifests, injected staging failure, injected publication failure with rollback, backup bytes and staging cleanup. CI also installs the actual built VST3, verifies its installed checksum, and checks explicit replacement. UAC prompts, locked plugins on a desktop, physical audio devices and FL Studio scans remain manual.

## Manual release gate

Follow FL-STUDIO-SETUP.md on Windows 10/11 and record the actual FL Studio version, device/driver, sample rate, buffer size, and results. Test plugin scanning as a generator, multiple instances, all styles/layers/effects, MIDI and WAV imports, exact state/history recall, stop/start/seeks, tempo/rate changes, resizing/high-DPI, offline DAW bounces, cancellation, replacement/backup recovery, and shutdown/removal. Export a SAVE REPORT file for any failure. Automated CI passing does not mark these checks complete.

Further work includes desktop profiling/host testing, note chasing, zoom/multi-selection, preset search/tags, richer musical generation, higher-quality stretching, signed distribution and broader installer UX. No feature or compatibility certification is implied for these pending items.
