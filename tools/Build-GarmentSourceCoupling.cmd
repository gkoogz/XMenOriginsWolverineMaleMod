@echo off
setlocal
rem Explicit Base checkout/library must match the pinned source worker receipt.
if "%~3"=="" exit /b 2
call "C:\BuildTools\VC\Auxiliary\Build\vcvars32.bat" >nul
cl /nologo /O2 /MT /EHsc /std:c++17 /I"%~1\include" /I"%~dp0..\third-party" /I"C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include" "%~dp0..\tests\garment_source_coupling_test.cpp" /Fe:"%~3" /Fo:"%~3.obj" /link "%~2" /LIBPATH:"C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Lib\x86" d3d9.lib d3dx9.lib d3d11.lib d3dcompiler.lib user32.lib gdi32.lib ole32.lib winmm.lib
exit /b %ERRORLEVEL%
