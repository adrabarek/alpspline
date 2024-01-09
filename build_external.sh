#!/usr/bin/env bash
set -e

mkdir -p bin/ext
mkdir -p src/ext

if [ ! -d "src/ext/raylib" ]; then
    git clone --branch "5.0" https://github.com/raysan5/raylib.git src/ext/raylib
fi

pushd src/ext/raylib/src
make clean
make -j3 PLATFORM=PLATFORM_DESKTOP
cp libraylib.a ../../../../bin/ext/
popd
