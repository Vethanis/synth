#pragma once

#include <cmath>
#include "constants.h"

// whiteish noise, [0, 1]
extern unsigned int g_randSeed;

inline float randf() 
{
    unsigned f = g_randSeed;

    f = (f ^ 61) ^ (f >> 16);
    f *= 9;
    f = f ^ (f >> 4);
    f *= 0x27d4eb2d;
    f = f ^ (f >> 15);

    g_randSeed = f;
    
    return (float)fract((double)f / double(0xffffffff));
}

inline float noise_wave(float)
{
    return randf() * 2.0f - 1.0f;
}

inline float saw_wave(float phase)
{
    phase /= tau;
    return lerp(-1.0f, 1.0f, phase);
}

inline float sine_wave(float phase)
{
    return sinf(phase);
}

inline float square_wave(float phase)
{
    return phase < pi ? 1.0f : -1.0f;
}

inline float triangle_wave(float phase)
{
    if(phase < pi)
        return lerp(-1.0f, 1.0f, phase / pi);
    
    phase -= pi;
    return lerp(1.0f, -1.0f, phase / pi);
}
