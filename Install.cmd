@echo off
setlocal
title Wolverine Anatomy Tool v0.7.1 Installer
powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%~dp0Install.ps1" -Mode Install
if errorlevel 1 (
  echo.
  echo Installation failed. No unsupported files were intentionally overwritten.
)
echo.
pause
