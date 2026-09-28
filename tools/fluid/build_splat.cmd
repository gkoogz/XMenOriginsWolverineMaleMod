@echo off
call "%~dp0..\..\src\runtime\build_splat_assets.cmd"
if errorlevel 1 exit /b 1
call "C:\BuildTools\VC\Auxiliary\Build\vcvars32.bat" >nul
cl /nologo /O2 /MT /EHsc /std:c++17 /I"C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include" splat_test.cpp ..\..\src\runtime\splat_bakes.res /link /OUT:splat-test.exe /LIBPATH:"C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Lib\x86" d3d9.lib d3dx9.lib user32.lib
