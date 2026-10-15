param([switch]$Replace)
$ErrorActionPreference = 'Stop'
$source = Join-Path $PSScriptRoot 'WEE.vst3'
$binary = Join-Path $source 'Contents/x86_64-win/WEE.vst3'
$destination = Join-Path ${env:CommonProgramFiles} 'VST3/WEE.vst3'
$backup = $null
try {
 if (![Environment]::Is64BitOperatingSystem) { throw 'WEE requires 64-bit Windows 10/11.' }
 if (!(Test-Path $binary)) { throw 'Extract the entire downloaded ZIP before installing.' }
 $principal = New-Object Security.Principal.WindowsPrincipal([Security.Principal.WindowsIdentity]::GetCurrent())
 if (!$principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) { throw 'Start Install-WEE.cmd to approve administrator access.' }
 if (Get-Process FL64,FL -ErrorAction SilentlyContinue) { throw 'Close FL Studio before installing WEE.' }
 if (Test-Path $destination) {
  if (!$Replace) {
   $answer = Read-Host 'WEE is already installed. Back up and replace it? Type YES'
   if ($answer -cne 'YES') { throw 'Installation cancelled; existing WEE was kept.' }
  }
  $backup = "$destination.backup-$(Get-Date -Format yyyyMMdd-HHmmss-fff)"
  Move-Item $destination $backup
 }
 New-Item (Split-Path $destination) -ItemType Directory -Force | Out-Null
 Copy-Item $source $destination -Recurse -ErrorAction Stop
 Write-Host "Installed WEE to $destination"
 Write-Host 'Open FL Studio > Options > Manage plugins > Find installed plugins. Add WEE as a generator.'
 if ($backup) { Write-Host "Previous installation: $backup" }
} catch {
 if ($backup -and (Test-Path $backup)) {
  if (Test-Path $destination) { Remove-Item $destination -Recurse -Force }
  Move-Item $backup $destination
 }
 Write-Error $_
 exit 1
}
