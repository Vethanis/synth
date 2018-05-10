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

#define eTriangleWave 0
#define eSawWave 1
#define eSquareWave 2
#define eNoiseWave 3
#define eWaveCount 4

inline float imeta_wave(int type, float phase)
{
    switch(type)
    {
        case 0: return triangle_wave(phase);
        case 1: return saw_wave(phase);
        case 2: return square_wave(phase);
        case 3: return noise_wave(phase);
    }
    return triangle_wave(phase);
}

inline float meta_wave(float type, float phase)
{
    const int ta = (int)floorf(type);
    const int tb = (int)ceilf(type);
    const float alpha = fract(type);
    const float A = imeta_wave(ta, phase);
    const float B = imeta_wave(tb, phase);
    return lerp(A, B, alpha);
}