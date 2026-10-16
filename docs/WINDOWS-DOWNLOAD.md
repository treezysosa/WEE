WEE — Windows x64 development build

1. Extract the ENTIRE WEE-Windows-x64 ZIP to a folder.
2. Close FL Studio.
3. Double-click Install-WEE.cmd and approve the Windows administrator prompt.
4. Open FL Studio > Options > Manage plugins > Find installed plugins.
5. Add WEE as an instrument/generator in the Channel Rack.

Manual installation: copy the entire WEE.vst3 folder into
C:\Program Files\Common Files\VST3\
Do not copy just the internal binary.

Double-click WEE.exe normally for standalone testing; administrator access is not required. Configure your audio device in Options > Audio/MIDI Settings. Version 0.5.1 embeds the MSVC runtime; a separate Visual C++ runtime installer is not expected. If no window appears, run Start-WEE.cmd. It keeps its console open and writes a local diagnostic file under your temporary WEE-startup folder with the process exit/window result and available crash events. Share only the failure message first; the complete local log may contain Windows account paths. It uploads nothing.
This development build is unsigned. SmartScreen may identify an unknown publisher.

Phase 5 adds optimized event scheduling and editor caching, glossy animated buttons, output meters, note tooltips, a local SAVE REPORT, and staged checksum-verified installation with rollback. Run Verify-WEE.cmd to verify the package and installed plugin. Retained backups can be restored manually; see PHASE-5.txt. FL Studio manual testing remains unverified.

Phase 3 includes real live DSP effects, background WAV/stem export (16/24-bit PCM and true float32), separate tails, and WEE FLIP audio transforms with cached original/flipped comparison. Use FX / FLIP / WAV. Main WAVs retain the selected bar length. RENDER A/B applies FLIP controls; Apply FLIP to WAV selects transformed exports.

Included from Phase 2: eight shared-harmony layers, eight synthesized instruments, nine styles, enable/mute/solo, volume/pan, layer locking and isolated regeneration, per-layer and multitrack MIDI export, complete generation undo/redo, host sync, automation and versioned project state. Only Lead is enabled initially; turn other layers ON. Click a layer name to inspect it.
Phase 4 adds EDIT / LIBRARY: transactional note editing, six factory presets and local .wee save/load, project-persistent bounded undo/redo, and phrase moods. Click empty space to add notes, drag to move, Shift-drag to resize, Alt-drag to set velocity, right-click/Delete to delete, Escape to cancel. Locks protect edits. Select a layer in CREATE first. Loading a preset is undoable. Advanced zoom/multi-selection and preset search remain future work. FL Studio manual compatibility is still unverified. Cached audition must be re-rendered after reopening or editing. FL Studio manual compatibility testing is still required.

To export: SAVE ALL MIDI then drag the saved .mid into FL Studio Piano Roll, or use Piano Roll's MIDI import. DRAG MIDI attempts native file dragging; saving to disk is the reliable fallback. Full export follows enable/mute/solo; each layer MIDI button exports its notes independently. Global generation refuses key/scale/style/length changes while notes are locked, preserving your existing music.

The GitHub Actions build runs automated composition/MIDI, audio and state tests. It does not run FL Studio. JUCE is licensed separately; see JUCE-LICENSE.md.

WAV export: select format, seamless and optional separate tail settings. SAVE WAV / EXPORT STEMS creates a new subfolder. Drag WAV attempts native file dragging; OPEN EXPORT FOLDER is the fallback. Half-time slows the first half to fill the same bars, one octave lower before transpose. Granular transpose retains duration. Seamless exports warm effects then use 2 ms boundary fades; slow delays may outlast a user-selected tail cutoff.
