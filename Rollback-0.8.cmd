@echo off
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0Upgrade-0.8.ps1" -Rollback %*
pause
