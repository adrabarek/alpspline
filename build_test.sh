set -e

clang++ -g3 -O0 -std=c++17 -c \
    -Isrc/alpspline/include \
    src/alpspline/src/alpspline.cpp \
    -o bin/alpspline.o
clang++ -g3 -O0 -std=c++17 -c \
    -Isrc/alpspline/include \
    src/test/test.cpp \
    -o bin/test.o
clang++ \
    bin/alpspline.o \
    bin/test.o \
    -o bin/test
bin/test
