#pragma once

#include <cmath>
#include "constants.h"

// whiteish noise, [0, 1]
inline float randf() {
    static int f = 41020057;
    f = (f ^ 61) ^ (f >> 16);
    f *= 9;
    f = f ^ (f >> 4);
    f *= 0x27d4eb2d;
    f = f ^ (f >> 15);
    return fract(float(f) * 2.3283064e-10f);
}

float saw_wave(float phase){
    phase /= tau;
    return lerp(-1.0f, 1.0f, phase);
}
float sine_wave(float phase){
    return sinf(phase);
}
float square_wave(float phase){
    return phase < pi ? 1.0f : -1.0f;
}
float triangle_wave(float phase){
    if(phase < pi){
        return lerp(-1.0f, 1.0f, phase / pi);
    }
    else{
        phase -= pi;
        return lerp(1.0f, -1.0f, phase / pi);
    }
}
