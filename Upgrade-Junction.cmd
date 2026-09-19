@echo off
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0Upgrade-Junction.ps1" %*
pause
