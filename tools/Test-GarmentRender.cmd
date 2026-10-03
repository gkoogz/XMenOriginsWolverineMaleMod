@echo off
setlocal
pwsh -NoProfile -File "%~dp0Resolve-Base.ps1" > "%~dp0..\build\garment-render-base.path"
if errorlevel 1 exit /b 1
set "MALEMOD_GARMENT_RENDER_BASE="
set /p MALEMOD_GARMENT_RENDER_BASE=<"%~dp0..\build\garment-render-base.path"
if not defined MALEMOD_GARMENT_RENDER_BASE exit /b 1
call "C:\BuildTools\VC\Auxiliary\Build\vcvars32.bat" >nul
cl /nologo /O2 /MT /EHsc /std:c++17 /I"%MALEMOD_GARMENT_RENDER_BASE%\include" /I"%~dp0..\third-party" /I"C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include" "%~dp0..\tests\garment_render_test.cpp" /Fe:"%~dp0..\build\garment_render_test.exe" /Fo:"%~dp0..\build\garment_render_test.obj" /link /LIBPATH:"C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Lib\x86" d3d9.lib d3dx9.lib d3d11.lib d3dcompiler.lib user32.lib gdi32.lib ole32.lib winmm.lib
if errorlevel 1 exit /b 1
"%~dp0..\build\garment_render_test.exe"
exit /b %ERRORLEVEL%
