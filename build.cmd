@echo off
set "MALEMOD_BASE_ARGS="
if /I "%~1"=="--diagnostic" set "MALEMOD_BASE_ARGS=-Diagnostic"
if not exist "%~dp0build" mkdir "%~dp0build"
pwsh -NoProfile -File "%~dp0tools\Resolve-Base.ps1" %MALEMOD_BASE_ARGS% > "%~dp0build\base.path"
if errorlevel 1 exit /b 1
set "MALEMOD_BASE_RESOLVED="
set /p MALEMOD_BASE_RESOLVED=<"%~dp0build\base.path"
if not defined MALEMOD_BASE_RESOLVED exit /b 1
call "C:\BuildTools\VC\Auxiliary\Build\vcvars32.bat" >nul
if not exist "%~dp0build" mkdir "%~dp0build"
pushd "%~dp0src\runtime"
rc /nologo /fo "..\..\build\splat_bakes.res" splat_bakes.rc
if errorlevel 1 (
 popd
 exit /b 1
)
cl.exe /nologo /LD /O2 /MT /EHsc /std:c++17 /I"%MALEMOD_BASE_RESOLVED%\include" /I"..\..\third-party" /I"C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include" d3d9_proxy.cpp "..\..\build\splat_bakes.res" /link /DEF:d3d9_proxy.def /OUT:..\..\build\d3d9.dll /LIBPATH:"%WindowsSdkDir%Lib\%WindowsSDKVersion%um\x86" /LIBPATH:"C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Lib\x86" d3d9.lib d3dx9.lib d3d11.lib d3dcompiler.lib user32.lib gdi32.lib ole32.lib winmm.lib
set buildResult=%ERRORLEVEL%
popd
exit /b %buildResult%
