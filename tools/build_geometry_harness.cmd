@echo off
call "C:\BuildTools\VC\Auxiliary\Build\vcvars32.bat" >nul
pushd "%~dp0"
cl.exe /nologo /O2 /EHsc /std:c++17 /DRUNTIME_SOURCE="\"../src/runtime/d3d9_proxy.cpp\"" /I"C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include" deterministic_harness.cpp /link /OUT:deterministic_harness.exe /LIBPATH:"C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Lib\x86" d3dx9.lib user32.lib gdi32.lib
set BUILD_RESULT=%ERRORLEVEL%
popd
exit /b %BUILD_RESULT%
