param([switch]$Replace)
$ErrorActionPreference='Stop'
. (Join-Path $PSScriptRoot 'InstallerCore.ps1')
if (![Environment]::Is64BitOperatingSystem) { throw 'WEE requires 64-bit Windows 10/11.' }
$principal=New-Object Security.Principal.WindowsPrincipal([Security.Principal.WindowsIdentity]::GetCurrent())
if (!$principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) { throw 'Run PowerShell as Administrator to install to Common Files.' }
if (Get-Process FL64,FL -ErrorAction SilentlyContinue) { throw 'Close FL Studio before installing WEE.' }
$root=Split-Path $PSScriptRoot -Parent
$source=Join-Path $root 'build/WEE_artefacts/Release/VST3/WEE.vst3'
$common64=if ($env:CommonProgramW6432) { $env:CommonProgramW6432 } else { $env:CommonProgramFiles }
$result=Install-WeeBundle -Source $source -Destination (Join-Path $common64 'VST3/WEE.vst3') -Replace:$Replace
Write-Host "Installed and verified $($result.Destination)"
if ($result.Backup) { Write-Host "Previous installation: $($result.Backup)" }
