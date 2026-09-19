@echo off
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0Upgrade-0.7.1.ps1" -Rollback %*
pause
