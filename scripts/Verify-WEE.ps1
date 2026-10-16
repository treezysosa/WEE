$ErrorActionPreference='Stop'
try {
 . (Join-Path $PSScriptRoot 'InstallerCore.ps1')
 Test-WeePackage -Directory $PSScriptRoot
 $source=Join-Path $PSScriptRoot 'WEE.vst3';$installed=Join-Path ${env:CommonProgramFiles} 'VST3/WEE.vst3'
 foreach ($file in (Get-ChildItem -LiteralPath $source -Recurse -File)) { $target=Join-Path $installed $file.FullName.Substring($source.Length+1);if (!(Test-Path -LiteralPath $target) -or (Get-FileHash -LiteralPath $target).Hash -ne (Get-FileHash -LiteralPath $file.FullName).Hash) { throw "Installed plugin differs or is missing: $target" } }
 Write-Host "PASS: package checksums and installed plugin match at $installed"
} catch { Write-Error $_;exit 1 }
