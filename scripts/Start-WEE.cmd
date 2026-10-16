@echo off
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0Start-WEE.ps1"
if errorlevel 1 echo WEE startup failed. Copy the diagnostic message above.
pause
