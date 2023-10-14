#!/usr/bin/env bash
set -e

# GUI libs unity build
clang++ -c \
    -Isrc/ext/imgui \
    -Isrc/ext/raylib \
    src/ext/imgui_all.cpp \
    -o bin/ext/imgui_all.o 
