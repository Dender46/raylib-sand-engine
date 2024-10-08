@echo off
setlocal

cd /D "%~dp0"

@REM g++ -std=c++20 -fdiagnostics-color=always -g -E ..\src\main.cpp  -I..\vendor\raylib\include -I..\vendor\box2d\include -L..\vendor\raylib\lib -L..\vendor\box2d\lib -l:libraylib.a -l:box2d.lib -lgdi32 -lwinmm > file.txt
g++ -std=c++20 -fdiagnostics-color=always -g ..\src\main.cpp -o ..\build\main.exe -I..\vendor\raylib\include -I..\vendor\box2d\include -L..\vendor\raylib\lib -L..\vendor\box2d\lib -l:libraylib.a -l:box2d.lib -lgdi32 -lwinmm > file.txt