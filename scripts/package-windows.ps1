$ErrorActionPreference = 'Stop'
Set-Location (Split-Path $PSScriptRoot -Parent)
$bundle = 'build/WEE_artefacts/Release/VST3/WEE.vst3'
$standalone = 'build/WEE_artefacts/Release/Standalone/WEE.exe'
if (!(Test-Path "$bundle/Contents/x86_64-win/WEE.vst3")) { throw 'Missing x64 VST3 binary' }
if (!(Test-Path $standalone)) { throw 'Missing standalone executable' }
$package = 'dist/WEE-Windows-x64'
New-Item $package -ItemType Directory -Force | Out-Null
Copy-Item $bundle "$package/WEE.vst3" -Recurse -Force
Copy-Item $standalone "$package/WEE.exe" -Force
Copy-Item 'scripts/Install-WEE.ps1' "$package/Install-WEE.ps1" -Force
Copy-Item 'scripts/Install-WEE.cmd' "$package/Install-WEE.cmd" -Force
Copy-Item 'docs/WINDOWS-DOWNLOAD.md' "$package/READ-ME.txt" -Force
Copy-Item 'LICENSE' "$package/LICENSE.txt" -Force
Copy-Item 'build/_deps/juce-src/LICENSE.md' "$package/JUCE-LICENSE.md" -Force
Get-ChildItem $package -Recurse -File | Get-FileHash -Algorithm SHA256 | ForEach-Object {
 '{0}  {1}' -f $_.Hash, $_.Path.Substring((Resolve-Path $package).Path.Length + 1)
} | Set-Content "$package/SHA256SUMS.txt"
Write-Host "Packaged $package"
