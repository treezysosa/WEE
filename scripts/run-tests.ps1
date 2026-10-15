$ErrorActionPreference = 'Stop'
Set-Location (Split-Path $PSScriptRoot -Parent)
& ctest --test-dir build -C Release --output-on-failure
if ($LASTEXITCODE) { throw 'Tests failed' }
