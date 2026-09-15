@echo off
call "C:\BuildTools\VC\Auxiliary\Build\vcvars32.bat" >nul
cl.exe /nologo /LD /O2 /EHsc /std:c++17 /I"C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include" d3d9_proxy.cpp /link /DEF:d3d9_proxy.def /OUT:d3d9.dll /LIBPATH:"C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Lib\x86" d3d9.lib d3dx9.lib user32.lib gdi32.lib
