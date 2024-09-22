@echo off
setlocal

cd /D "%~dp0"

pushd build

set FLAGS= /nologo /EHsc /Zi /MD /std:c++20
set COMPILATION_FILES= ..\src\main.cpp ..\src\profiler\profiller.cpp
set INCLUDES= /I..\vendor\raylib\include /I..\vendor\box2d\include
set LIBS= ..\vendor\raylib\lib\raylib.lib ..\vendor\box2d\lib\box2d.lib opengl32.lib kernel32.lib user32.lib shell32.lib gdi32.lib winmm.lib msvcrt.lib

cl %FLAGS% %INCLUDES% %COMPILATION_FILES% %LIBS% /link /out:main.exe /NODEFAULTLIB:libcmt

popd
