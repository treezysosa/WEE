param([switch]$Replace)
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$source = Join-Path $root 'build\WEE_artefacts\Release\VST3\WEE.vst3'
$destination = Join-Path $env:CommonProgramFiles 'VST3\WEE.vst3'
if (!(Test-Path $source)) { throw "Build bundle missing: $source" }
$principal = New-Object Security.Principal.WindowsPrincipal([Security.Principal.WindowsIdentity]::GetCurrent())
if (!$principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) { throw 'Run PowerShell as Administrator to install to Common Files.' }
if (Test-Path $destination) {
 if (!$Replace) { throw 'Existing installation found. Close FL Studio and use -Replace to back up and replace it.' }
 Move-Item $destination "$destination.backup-$(Get-Date -Format yyyyMMdd-HHmmss)"
}
try { New-Item (Split-Path $destination) -ItemType Directory -Force | Out-Null; Copy-Item $source $destination -Recurse; Write-Host "Installed $destination" }
catch { throw "Installation failed: $_. Any previous bundle remains in its timestamped backup." }
