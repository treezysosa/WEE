@echo off
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0Verify-WEE.ps1"
if errorlevel 1 (echo Verification failed. Read the message above.) else (echo Package and installed plugin verified.)
pause
