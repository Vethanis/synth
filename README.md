# audiostreaming

A simple UDP audio streaming program

## Usage

./main.exe *ipv4_address* *port* *packet_buffer_length(0-7)* *uploading/downloading(u/d)*

ie. ./main.exe 127.0.0.1 8000 2 u

## Dependencies

* C++11 compiler
* CMake 3.1 or greater
* RtAudio is included

## Building

* use CMake, or ./compile.sh
* mkdir build && cd build
* cmake .. 
* cd .. && cmake --build build --config Release

