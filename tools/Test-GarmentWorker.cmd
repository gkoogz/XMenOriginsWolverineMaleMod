@echo off
setlocal
if not exist "%~dp0..\build" mkdir "%~dp0..\build"
pwsh -NoProfile -File "%~dp0Resolve-Base.ps1" > "%~dp0..\build\garment-test-base.path"
if errorlevel 1 exit /b 1
set "MALEMOD_GARMENT_TEST_BASE="
set /p MALEMOD_GARMENT_TEST_BASE=<"%~dp0..\build\garment-test-base.path"
if not defined MALEMOD_GARMENT_TEST_BASE exit /b 1
call "C:\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul
cl /nologo /O2 /MT /EHsc /std:c++17 /I"%MALEMOD_GARMENT_TEST_BASE%\include" "%~dp0..\tests\garment_worker_test.cpp" /Fe:"%~dp0..\build\garment_worker_test.exe" /Fo:"%~dp0..\build\garment_worker_test.obj"
if errorlevel 1 exit /b 1
"%~dp0..\build\garment_worker_test.exe"
if errorlevel 1 exit /b 1
cl /nologo /O2 /MT /EHsc /std:c++17 /I"%MALEMOD_GARMENT_TEST_BASE%\include" "%~dp0..\tests\garment_worker_reaction_test.cpp" /Fe:"%~dp0..\build\garment_worker_reaction_test.exe" /Fo:"%~dp0..\build\garment_worker_reaction_test.obj"
if errorlevel 1 exit /b 1
"%~dp0..\build\garment_worker_reaction_test.exe"
exit /b %ERRORLEVEL%
