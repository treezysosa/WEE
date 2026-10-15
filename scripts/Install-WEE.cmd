@echo off
set "WEE_INSTALL_SCRIPT=%~dp0Install-WEE.ps1"
powershell.exe -NoProfile -ExecutionPolicy Bypass -Command "$p = Start-Process powershell.exe -Verb RunAs -Wait -PassThru -ArgumentList ('-NoProfile -ExecutionPolicy Bypass -File ""' + $env:WEE_INSTALL_SCRIPT + '""'); exit $p.ExitCode"
if errorlevel 1 (
 echo WEE installation failed or was cancelled.
) else (
 echo WEE installation completed. Scan plugins in FL Studio.
)
pause
