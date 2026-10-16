# Shared by the administrator installer and isolated installer tests. No elevation or process launch here.
Set-StrictMode -Version Latest
function Test-WeePackage {
 param([Parameter(Mandatory)][string]$Directory)
 $root = [IO.Path]::GetFullPath($Directory).TrimEnd('\','/')
 $manifest = Join-Path $root 'SHA256SUMS.txt'
 if (!(Test-Path -LiteralPath $manifest -PathType Leaf)) { throw 'Missing SHA256SUMS.txt. Extract the entire package.' }
 $entries = @{}; $lines = @(Get-Content -LiteralPath $manifest -Encoding UTF8)
 if ($lines.Count -eq 0 -or $lines.Count -gt 1024) { throw 'Invalid checksum manifest size.' }
 foreach ($line in $lines) {
  if ($line -notmatch '^([0-9A-Fa-f]{64})  (.+)$') { throw 'Invalid checksum manifest entry.' }
  $hash = $Matches[1]; $relative = $Matches[2]
  if ([IO.Path]::IsPathRooted($relative) -or $relative -match '(^|[\\/])\.\.([\\/]|$)' -or $relative.Contains(':')) { throw 'Unsafe checksum path.' }
  $file = [IO.Path]::GetFullPath((Join-Path $root $relative))
  if (!$file.StartsWith($root + [IO.Path]::DirectorySeparatorChar,[StringComparison]::OrdinalIgnoreCase) -or $entries.ContainsKey($file)) { throw 'Duplicate or unsafe checksum path.' }
  if (!(Test-Path -LiteralPath $file -PathType Leaf) -or (Get-FileHash -LiteralPath $file -Algorithm SHA256).Hash -ne $hash) { throw "Package verification failed: $relative" }
  $entries[$file]=$hash
 }
 foreach ($file in (Get-ChildItem -LiteralPath (Join-Path $root 'WEE.vst3') -File -Recurse)) { if (!$entries.ContainsKey($file.FullName)) { throw 'Plugin contains a file missing from the manifest.' } }
 if (!$entries.ContainsKey([IO.Path]::GetFullPath((Join-Path $root 'WEE.vst3/Contents/x86_64-win/WEE.vst3')))) { throw 'Manifest does not cover the x64 plugin.' }
}
function Install-WeeBundle {
 param([Parameter(Mandatory)][string]$Source,[Parameter(Mandatory)][string]$Destination,[switch]$Replace)
 $Source=[IO.Path]::GetFullPath($Source).TrimEnd('\','/');$Destination=[IO.Path]::GetFullPath($Destination).TrimEnd('\','/')
 if ($Source -eq $Destination -or !(Test-Path -LiteralPath (Join-Path $Source 'Contents/x86_64-win/WEE.vst3'))) { throw 'Invalid source plugin bundle.' }
 if ((Test-Path -LiteralPath $Destination) -and !$Replace) { throw 'Existing plugin preserved. Replacement requires explicit permission.' }
 $parent=Split-Path $Destination;New-Item -Path $parent -ItemType Directory -Force | Out-Null
 $stage=Join-Path $parent ('.WEE-install-'+[guid]::NewGuid().ToString('N'));$backup=$null;$published=$false
 try {
  Copy-Item -LiteralPath $Source -Destination $stage -Recurse -ErrorAction Stop
  $files=@(Get-ChildItem -LiteralPath $Source -Recurse -File);$staged=@(Get-ChildItem -LiteralPath $stage -Recurse -File)
  if ($files.Count -ne $staged.Count) { throw 'Staged plugin file count differs.' }
  foreach ($file in $files) { $relative=$file.FullName.Substring($Source.Length+1);$copy=Join-Path $stage $relative;if (!(Test-Path -LiteralPath $copy) -or (Get-FileHash -LiteralPath $file.FullName).Hash -ne (Get-FileHash -LiteralPath $copy).Hash) { throw 'Staged plugin checksum mismatch.' } }
  if (Test-Path -LiteralPath $Destination) { $backup=$Destination+'.backup-'+(Get-Date -Format yyyyMMdd-HHmmss-fff)+'-'+[guid]::NewGuid().ToString('N').Substring(0,6);Move-Item -LiteralPath $Destination -Destination $backup -ErrorAction Stop }
  Move-Item -LiteralPath $stage -Destination $Destination -ErrorAction Stop;$published=$true
  foreach ($file in $files) { $copy=Join-Path $Destination $file.FullName.Substring($Source.Length+1);if ((Get-FileHash -LiteralPath $copy).Hash -ne (Get-FileHash -LiteralPath $file.FullName).Hash) { throw 'Installed plugin checksum mismatch.' } }
  [pscustomobject]@{Destination=$Destination;Backup=$backup;Verified=$true}
 } catch {
  $failure=$_
  if ($backup -and (Test-Path -LiteralPath $backup)) {
   try { if ($published -and (Test-Path -LiteralPath $Destination)) { Remove-Item -LiteralPath $Destination -Recurse -Force -ErrorAction Stop };if (Test-Path -LiteralPath $Destination) { throw 'Destination is occupied.' };Move-Item -LiteralPath $backup -Destination $Destination -ErrorAction Stop }
   catch { throw "Installation failed and automatic rollback could not finish. Previous plugin remains at $backup. Close audio hosts and restore it manually. Original error: $failure. Rollback: $_" }
  } elseif ($published -and (Test-Path -LiteralPath $Destination)) { Remove-Item -LiteralPath $Destination -Recurse -Force -ErrorAction SilentlyContinue }
  throw $failure
 } finally { if (Test-Path -LiteralPath $stage) { Remove-Item -LiteralPath $stage -Recurse -Force -ErrorAction SilentlyContinue } }
}
