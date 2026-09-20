@echo off
if not exist tests mkdir tests
call "C:\BuildTools\VC\Auxiliary\Build\vcvars32.bat" >nul
cl.exe /nologo /O2 /EHsc /std:c++17 /I"C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include" capture_test.cpp /link /OUT:tests/capture_test.exe /LIBPATH:"C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Lib\x86" d3d9.lib d3dx9.lib user32.lib gdi32.lib



