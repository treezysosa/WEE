$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
$out=Join-Path $root 'build/startup-diagnostics'
& (Join-Path $root 'scripts/Start-WEE.ps1') -Executable (Join-Path $root 'build/WEE_artefacts/Release/Standalone/WEE.exe') -SmokeTest -LogDirectory $out
if ($LASTEXITCODE -ne 0) { throw 'Standalone startup smoke test failed.' }
$plugin=Join-Path $root 'build/WEE_artefacts/Release/VST3/WEE.vst3/Contents/x86_64-win/WEE.vst3'
$module=[WeeStartupNative]::LoadLibraryEx($plugin,[IntPtr]::Zero,0x1100)
if ($module -eq [IntPtr]::Zero) { throw ('VST3 module load failed with Windows error '+[Runtime.InteropServices.Marshal]::GetLastWin32Error()) }
try { if ([WeeStartupNative]::GetProcAddress($module,'GetPluginFactory') -eq [IntPtr]::Zero) { throw 'VST3 factory export missing.' };Write-Host 'PASS: Windows loaded the VST3 module and found GetPluginFactory (not a DAW compatibility test).' }
finally { [void][WeeStartupNative]::FreeLibrary($module) }
