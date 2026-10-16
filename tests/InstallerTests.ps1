$ErrorActionPreference='Stop'
. (Join-Path $PSScriptRoot '../scripts/InstallerCore.ps1')
function Assert([bool]$Condition,[string]$Message) { if (!$Condition) { throw $Message } }
$root=Join-Path ([IO.Path]::GetTempPath()) ('wee-install-tests-'+[guid]::NewGuid().ToString('N'))
$source=Join-Path $root 'package/WEE.vst3';$binary=Join-Path $source 'Contents/x86_64-win/WEE.vst3';$destination=Join-Path $root 'installed/WEE.vst3'
try {
 New-Item -ItemType Directory -Force (Split-Path $binary) | Out-Null;[IO.File]::WriteAllText($binary,'original test binary')
 $manifest=Join-Path $root 'package/SHA256SUMS.txt';((Get-WeeFileHash $binary).Hash+'  WEE.vst3/Contents/x86_64-win/WEE.vst3') | Set-Content $manifest -Encoding UTF8
 Test-WeePackage (Join-Path $root 'package')
 $result=Install-WeeBundle $source $destination;Assert $result.Verified 'Fresh install failed';$old=(Get-WeeFileHash (Join-Path $destination 'Contents/x86_64-win/WEE.vst3')).Hash
 $rejected=$false;try { Install-WeeBundle $source $destination | Out-Null } catch { $rejected=$true };Assert $rejected 'Unapproved replacement accepted'
 [IO.File]::WriteAllText($binary,'new version');$rejected=$false;try { Test-WeePackage (Join-Path $root 'package') } catch { $rejected=$true };Assert $rejected 'Corruption was accepted'
 # Inject a staging copy failure; the previous installation must never move.
 function Copy-Item { param($LiteralPath,$Destination,[switch]$Recurse,$ErrorAction);throw 'Injected staging copy failure' }
 $rejected=$false;try { Install-WeeBundle $source $destination -Replace | Out-Null } catch { $rejected=$true };Remove-Item Function:Copy-Item
 Assert $rejected 'Staging failure was ignored';Assert ((Get-WeeFileHash (Join-Path $destination 'Contents/x86_64-win/WEE.vst3')).Hash -eq $old) 'Staging failure changed previous plugin'
 # Fail the publication rename after backup; rollback must restore the old bytes.
 function Move-Item { param($LiteralPath,$Destination,$ErrorAction);if ((Split-Path $LiteralPath -Leaf).StartsWith('.WEE-install-')) { throw 'Injected publish failure' };Microsoft.PowerShell.Management\Move-Item -LiteralPath $LiteralPath -Destination $Destination -ErrorAction Stop }
 $rejected=$false;try { Install-WeeBundle $source $destination -Replace | Out-Null } catch { $rejected=$true };Remove-Item Function:Move-Item
 Assert $rejected 'Publication failure was ignored';Assert ((Get-WeeFileHash (Join-Path $destination 'Contents/x86_64-win/WEE.vst3')).Hash -eq $old) 'Rollback failed to restore previous plugin'
 Assert (@(Get-ChildItem (Split-Path $destination) -Force -Filter '.WEE-install-*').Count -eq 0) 'Staging folder leaked'
 $result=Install-WeeBundle $source $destination -Replace;Assert (Test-Path $result.Backup) 'Successful replacement lost backup';Assert ((Get-WeeFileHash (Join-Path $result.Backup 'Contents/x86_64-win/WEE.vst3')).Hash -eq $old) 'Backup bytes differ'
 # Traversal entries are refused before reading outside the package.
 ('0'*64+'  ../outside.dll') | Set-Content $manifest -Encoding UTF8;$rejected=$false;try { Test-WeePackage (Join-Path $root 'package') } catch { $rejected=$true };Assert $rejected 'Unsafe manifest accepted'
 Write-Host 'PASS: package integrity, fresh install, explicit replacement, staging failure, rollback, backup and cleanup'
} finally { Remove-Item $root -Recurse -Force -ErrorAction SilentlyContinue }
