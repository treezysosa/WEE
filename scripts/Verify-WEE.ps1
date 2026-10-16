$ErrorActionPreference='Stop'
try {
 . (Join-Path $PSScriptRoot 'InstallerCore.ps1')
 Test-WeePackage -Directory $PSScriptRoot
 $source=Join-Path $PSScriptRoot 'WEE.vst3';$common64=if ($env:CommonProgramW6432) { $env:CommonProgramW6432 } else { $env:CommonProgramFiles };$installed=Join-Path $common64 'VST3/WEE.vst3'
 foreach ($file in (Get-ChildItem -LiteralPath $source -Recurse -File)) { $target=Join-Path $installed $file.FullName.Substring($source.Length+1);if (!(Test-Path -LiteralPath $target) -or (Get-WeeFileHash -LiteralPath $target).Hash -ne (Get-WeeFileHash -LiteralPath $file.FullName).Hash) { throw "Installed plugin differs or is missing: $target" } }
 Write-Host "PASS: package checksums and installed plugin match at $installed"
} catch { Write-Error $_;exit 1 }
