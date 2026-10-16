param(
 [string]$Executable=(Join-Path $PSScriptRoot 'WEE.exe'),
 [switch]$SmokeTest,
 [int]$TimeoutSeconds=30,
 [string]$LogDirectory=(Join-Path ([IO.Path]::GetTempPath()) 'WEE-startup')
)
$ErrorActionPreference='Stop'
New-Item -ItemType Directory -Force -Path $LogDirectory | Out-Null
$log=Join-Path $LogDirectory ('startup-'+[DateTime]::UtcNow.ToString('yyyyMMdd-HHmmss-fff')+'.txt')
function Record([string]$Text) { $line=[DateTime]::UtcNow.ToString('o')+' '+$Text;[IO.File]::AppendAllText($log,$line+[Environment]::NewLine);Write-Host $Text }
Add-Type -TypeDefinition @'
using System;
using System.Runtime.InteropServices;
public static class WeeStartupNative {
 [DllImport("user32.dll",SetLastError=true)] public static extern IntPtr SendMessageTimeout(IntPtr window,uint message,IntPtr wParam,IntPtr lParam,uint flags,uint timeout,out IntPtr result);
 [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr window);
 [DllImport("kernel32.dll",CharSet=CharSet.Unicode,SetLastError=true)] public static extern IntPtr LoadLibraryEx(string file,IntPtr reserved,uint flags);
 [DllImport("kernel32.dll",CharSet=CharSet.Ansi,SetLastError=true)] public static extern IntPtr GetProcAddress(IntPtr module,string name);
 [DllImport("kernel32.dll")] public static extern bool FreeLibrary(IntPtr module);
}
'@
$process=$null;$started=Get-Date
try {
 Record ('Windows: '+[Environment]::OSVersion.VersionString+'; OS64='+[Environment]::Is64BitOperatingSystem+'; process64='+[Environment]::Is64BitProcess)
 Record ('Executable: '+$Executable)
 if (!(Test-Path -LiteralPath $Executable -PathType Leaf)) { throw 'WEE.exe is missing. Extract the entire ZIP before using Start-WEE.cmd.' }
 if (![Environment]::Is64BitOperatingSystem) { throw 'This build requires 64-bit Windows 10/11.' }
 $Executable=[IO.Path]::GetFullPath($Executable)
 Record ('Size: '+(Get-Item -LiteralPath $Executable).Length+' bytes')
 $process=Start-Process -FilePath $Executable -WorkingDirectory (Split-Path $Executable) -PassThru
 Record ('Started process '+$process.Id)
 $deadline=[DateTime]::UtcNow.AddSeconds($TimeoutSeconds);$window=[IntPtr]::Zero
 while ([DateTime]::UtcNow -lt $deadline) {
  $process.Refresh()
  if ($process.HasExited) { $exit=$process.ExitCode;$hex=([BitConverter]::ToUInt32([BitConverter]::GetBytes([int]$exit),0)).ToString('X8');throw ('WEE exited before opening a window. Exit code '+$exit+' (0x'+$hex+').') }
  if ($process.MainWindowHandle -ne [IntPtr]::Zero -and [WeeStartupNative]::IsWindowVisible($process.MainWindowHandle)) { $window=$process.MainWindowHandle;break }
  Start-Sleep -Milliseconds 250
 }
 if ($window -eq [IntPtr]::Zero) { throw 'WEE is still running but has not shown a visible window. Startup may be waiting on an audio driver or another startup operation. The process was left running.' }
 Record ('Visible window: '+$process.MainWindowTitle+'; handle='+$window)
 $result=[IntPtr]::Zero
 if ([WeeStartupNative]::SendMessageTimeout($window,0,[IntPtr]::Zero,[IntPtr]::Zero,2,5000,[ref]$result) -eq [IntPtr]::Zero) { throw 'The WEE window is not responding to Windows messages.' }
 Record 'PASS: WEE opened a visible, responsive window.'
 if ($SmokeTest) {
  Start-Sleep -Seconds 3;$process.Refresh();if ($process.HasExited) { throw 'WEE exited during the startup observation period.' }
  if (!$process.CloseMainWindow() -or !$process.WaitForExit(10000)) { throw 'WEE did not close normally.' }
  if ($process.ExitCode -ne 0) { throw ('WEE shutdown exit code '+$process.ExitCode) }
  Record 'PASS: standalone remained running and closed normally.'
 } else { Record 'WEE was left open. Choose Options > Audio/MIDI Settings if you need to select an audio device.' }
} catch {
 Record ('FAIL: '+$_.Exception.Message)
 # Crash details are local only; failures to read the event log never hide the launch result.
 try { Get-WinEvent -FilterHashtable @{LogName='Application';StartTime=$started;Id=1000,1001} -MaxEvents 8 -ErrorAction Stop | Where-Object { $_.Message -match '(?i)wee\.exe' } | ForEach-Object { Record ('Windows event: '+$_.Message) } } catch {}
 Write-Host ('Diagnostic log: '+$log)
 if ($SmokeTest -and $process -and !$process.HasExited) { $process.Kill() }
 exit 1
} finally { if ($process) { $process.Dispose() } }
Write-Host ('Diagnostic log: '+$log)

exit 0
