@echo off
call "C:\BuildTools\VC\Auxiliary\Build\vcvars32.bat" >nul
cl /nologo /O2 /MT /EHsc /std:c++17 solver_test.cpp /link d3d11.lib d3dcompiler.lib /OUT:solver-test.exe
