@echo off

pushd %~dp0

if not exist vendor\ (
    mkdir vendor
)

if not exist vendor\raylib\ (
    powershell Invoke-WebRequest https://github.com/raysan5/raylib/releases/download/5.0/raylib-5.0_win64_mingw-w64.zip -OutFile vendor/raylib.zip
    echo Raylib: downloaded
    powershell Expand-Archive vendor/raylib.zip -DestinationPath vendor
    echo Raylib: extracted .zip file
    ren vendor\raylib-5.0_win64_mingw-w64 raylib
    del vendor\raylib.zip
    echo Raylib: renamed and cleaned up
) else (
    echo Raylib already exists in .\vendor\
)

echo Script completed
popd