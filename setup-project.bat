@echo off

pushd %~dp0

if not exist vendor\ (
    mkdir vendor
)

if not exist vendor\raylib\ (
    echo ===== Raylib: downloading raylib binaries and headers
    powershell Invoke-WebRequest https://github.com/raysan5/raylib/releases/download/5.0/raylib-5.0_win64_mingw-w64.zip -OutFile vendor/raylib.zip

    echo ===== Raylib: extracting
    powershell Expand-Archive vendor/raylib.zip -DestinationPath vendor

    echo ===== Raylib: renaming and cleaning up
    ren vendor\raylib-5.0_win64_mingw-w64 raylib
    del vendor\raylib.zip

    echo ===== Raylib: downloading additional headers
    echo       raygui.h
    powershell Invoke-WebRequest https://github.com/raysan5/raygui/raw/4.0/src/raygui.h -OutFile vendor/raylib/include/raygui.h
) else (
    echo Raylib already exists in .\vendor\
)

if not exist vendor\box2d\ (
    echo ===== Box2d: cloning repo
    git clone https://github.com/erincatto/box2d.git vendor\box2d

    pushd vendor\box2d
        echo ===== Box2d: cmake building
        mkdir build && cd build
        cmake ..
        cmake --build . --config Release

        echo ===== Box2d: moving lib
        cd ..
        mkdir lib
        move build\bin\Release\box2d.lib lib\

        echo ===== Box2d: cleaning up cmake build
        del   /s /q build
        rmdir /s /q build
    popd
) else (
    echo Box2d already exists in .\vendor\
)

echo Script completed
popd