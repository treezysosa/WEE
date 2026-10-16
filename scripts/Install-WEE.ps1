param([switch]$Replace)
$ErrorActionPreference = 'Stop'
try {
 . (Join-Path $PSScriptRoot 'InstallerCore.ps1')
 if (![Environment]::Is64BitOperatingSystem) { throw 'WEE requires 64-bit Windows 10/11.' }
 $principal = New-Object Security.Principal.WindowsPrincipal([Security.Principal.WindowsIdentity]::GetCurrent())
 if (!$principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) { throw 'Start Install-WEE.cmd to approve administrator access.' }
 if (Get-Process FL64,FL -ErrorAction SilentlyContinue) { throw 'Close FL Studio before installing WEE.' }
 Test-WeePackage -Directory $PSScriptRoot
 $common64=if ($env:CommonProgramW6432) { $env:CommonProgramW6432 } else { $env:CommonProgramFiles };$destination=Join-Path $common64 'VST3/WEE.vst3'
 if ((Test-Path -LiteralPath $destination) -and !$Replace) { $answer=Read-Host 'WEE is already installed. Back up and replace it? Type YES';if ($answer -cne 'YES') { throw 'Installation cancelled; existing WEE was kept.' };$Replace=$true }
 $result=Install-WeeBundle -Source (Join-Path $PSScriptRoot 'WEE.vst3') -Destination $destination -Replace:$Replace
 Write-Host "Installed and verified WEE at $($result.Destination)"
 if ($result.Backup) { Write-Host "Previous installation: $($result.Backup)" }
 Write-Host 'Open FL Studio > Options > Manage plugins > Find installed plugins. Add WEE as a generator.'
} catch { Write-Error $_;exit 1 }
