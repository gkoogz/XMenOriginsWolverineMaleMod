@echo off
setlocal
pushd "%~dp0"
call "C:\BuildTools\VC\Auxiliary\Build\vcvars32.bat" >nul
cl /nologo /O2 /MT /EHsc /std:c++17 /I"C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include" compile_splat_shader.cpp /link /OUT:compile-splat-shader.exe /LIBPATH:"C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Lib\x86" d3d9.lib d3dx9.lib user32.lib
if errorlevel 1 goto fail
compile-splat-shader.exe "..\..\src\runtime\stain_shader_bytecode.h.tmp"
if errorlevel 1 goto fail
copy /y "..\..\src\runtime\stain_shader_bytecode.h.tmp" "..\..\src\runtime\stain_shader_bytecode.h" >nul
if errorlevel 1 goto fail
popd
exit /b 0
:fail
popd
exit /b 1
