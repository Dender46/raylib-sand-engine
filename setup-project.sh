#!/bin/bash

# push dir where script is located, output of command is silenced
pushd $(dirname "$0") > /dev/null

test ! -d "vendor/" && mkdir vendor/

if [ ! -d "vendor/raylib/" ]; then
    cd vendor
        echo "===== Raylib: downloading raylib binaries and headers"
        # -L1 option is required for https
        curl -o raylib.tar.gz -L1 https://github.com/raysan5/raylib/releases/download/5.0/raylib-5.0_macos.tar.gz

        echo "===== Raylib: extracting"
        # --directory is not required
        tar -xf raylib.tar.gz --directory .

        echo "===== Raylib: renaming and cleaning up"
        mv raylib-5.0_macos raylib
        rm raylib.tar.gz

        echo "===== Raylib: downloading additional headers"
        cd raylib/include
            curl -o raygui.h -L1 https://github.com/raysan5/raygui/raw/4.0/src/raygui.h
        cd ../..
    cd ..
else
    echo "Raylib already exists in ./vendor/"
fi

exit
// TODO: box2d 

if not exist vendor\box2d\ (
    echo ===== Box2d: cloning repo
    git clone https://github.com/erincatto/box2d.git vendor\box2d

    pushd vendor\box2d
        echo ===== Box2d: cmake building
        mkdir build && cd build
        cmake ..
        cmake --build . --config Release

        echo =====Box2d: moving lib
        cd ..
        mkdir lib
        move build\bin\Release\box2d.lib lib\

        echo =====Box2d: cleaning up cmake build
        del   /s /q build
        rmdir /s /q build
    popd
) else (
    echo Box2d already exists in .\vendor\
)

echo Script completed
popd > /dev/null
