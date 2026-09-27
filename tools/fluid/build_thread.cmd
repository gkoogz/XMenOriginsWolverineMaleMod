@echo off
call "C:\BuildTools\VC\Auxiliary\Build\vcvars32.bat" >nul
cl /nologo /O2 /MT /EHsc /std:c++17 thread_test.cpp /link /OUT:thread-test.exe
