#!/usr/bin/env bash
set -e

mkdir -p bin/
clang++ -g3 -O0 -std=c++17 -c \
    -Isrc/include \
    -Isrc/ext/raylib \
    -Isrc/ext/imgui \
    -Isrc/ext/rlImGui \
    src/demo/unity.cpp \
    -o bin/demo.o
clang++ \
    bin/demo.o \
    bin/ext/imgui_all.o \
    -Lbin/ext -lraylib -lpthread -ldl \
    -o bin/demo
cp -rv src/resources bin/resources
pushd bin/
./demo
popd
