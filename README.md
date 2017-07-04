# audiostreaming

A simple UDP audio streaming program

## Usage

./main.exe *ipv4 address* *port* *packet buffer length 0-7* *uploading or downloading u/d*

## Dependencies

* C++11 compiler
* CMake 3.1 or greater
* RtAudio is included

## Building

* use CMake, or ./compile.sh
* mkdir build && cd build
* cmake .. 
* cd .. && cmake --build build --config Release

