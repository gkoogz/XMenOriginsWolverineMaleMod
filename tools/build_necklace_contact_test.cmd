@echo off
call "C:\BuildTools\VC\Auxiliary\Build\vcvars32.bat" >nul
cl.exe /nologo /O2 /EHsc /std:c++17 necklace_contact_test.cpp /Fe:necklace_contact_test.exe
