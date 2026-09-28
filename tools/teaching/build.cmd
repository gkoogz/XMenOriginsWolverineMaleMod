@echo off
call "%~dp0..\..\src\runtime\build_splat_assets.cmd"
if errorlevel 1 exit /b 1
call "C:\BuildTools\VC\Auxiliary\Build\vcvars32.bat" >nul
cl.exe /nologo /O2 /MT /EHsc /std:c++17 /I"C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include" sequence_test.cpp ..\..\src\runtime\splat_bakes.res /link /OUT:sequence-test.exe /LIBPATH:"%WindowsSdkDir%Lib\%WindowsSDKVersion%um\x86" /LIBPATH:"C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Lib\x86" d3d9.lib d3dx9.lib d3d11.lib d3dcompiler.lib user32.lib gdi32.lib ole32.lib winmm.lib
