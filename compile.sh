#!/usr/bin/env bash

if [[ ! -d build ]]; then
    mkdir build 
    cd build

    if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" ]]; then 
        if [[ $(uname -m) == "x86_64" ]]; then
            cmake .. -G "Visual Studio 15 Win64"
        else
            cmake .. -G "Visual Studio 15"
        fi
    else
        cmake ..
    fi

    cd ..
fi

if [[ $1 == "Debug" ]]; then
    cmake --build build --config Debug
else
    cmake --build build --config Release
fi

if [ $? == 0 ]; then
    ./bin/Release/main.exe
fi