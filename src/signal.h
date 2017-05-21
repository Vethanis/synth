#pragma once

#include <cstring>

enum SignalMode{
    SIGNAL_ADD,
    SIGNAL_SUB,
    SIGNAL_MUL,
    SIGNAL_DIV
};

struct Signal{
    float* src;
    float* dest;
    float amplitude;
    SignalMode mode;
    Signal() : src(nullptr), dest(nullptr), amplitude(1.0f), mode(SIGNAL_ADD){
    }
    inline void step(){
        if(src && dest){
            switch(mode){
                case SIGNAL_ADD:
                    *dest += *src * amplitude;
                break;
                case SIGNAL_SUB:
                    *dest -= *src * amplitude;
                break;
                case SIGNAL_MUL:
                    *dest *= *src * amplitude;
                break;
                case SIGNAL_DIV:
                    *dest /= *src * amplitude;
                break;
                default:
                    return;
            }
        }
    }
};
