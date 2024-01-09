#!/usr/bin/env bash
set -e

mkdir -p bin/
clang++ -g3 -O0 -std=c++17 -c \
    -Isrc/demo/include \
    -Isrc/alpspline/include \
    -Isrc/ext/raylib/src \
    src/demo/unity.cpp \
    -o bin/demo.o
clang++ -g3 -O0 -std=c++17 -c \
    -Isrc/alpspline/include \
    src/alpspline/src/alpspline.cpp \
    -o bin/alpspline.o
clang++ \
    bin/demo.o \
    bin/alpspline.o \
    -Lbin/ext -lraylib -lpthread -ldl \
    -o bin/demo
cp -rv src/resources bin/
pushd bin/
./demo
popd
