param([string]$JuceSourceDir = "")
$ErrorActionPreference = 'Stop'
Set-Location (Split-Path $PSScriptRoot -Parent)
$argsList = @('-S','.', '-B','build','-G','Visual Studio 17 2022','-A','x64')
if ($JuceSourceDir) { $argsList += "-DJUCE_SOURCE_DIR=$JuceSourceDir" }
& cmake @argsList
if ($LASTEXITCODE) { throw 'CMake configuration failed' }
& cmake --build build --config Release --parallel
if ($LASTEXITCODE) { throw 'Build failed' }
& cmake --build build --config Release --target wee_tests
& ctest --test-dir build -C Release --output-on-failure
if ($LASTEXITCODE) { throw 'Tests failed' }
Write-Host "VST3: $PWD\build\WEE_artefacts\Release\VST3\WEE.vst3"
