#pragma once

#define min(a, b)((a) < (b) ? (a) : (b))
#define max(a, b)((a) > (b) ? (a) : (b))
#define clamp(x, a, b)(min(max(x, a), b))
#define lerp(a, b, alpha)( (1.0f - (alpha)) * (a) + (alpha) * (b) )
#define fract(a)( (a) - float(int((a))) )

constexpr int sample_rate = 44100;
constexpr float inv_sample_rate = 1.0f / sample_rate;
constexpr float tau = 6.2831853f;
constexpr float pi = tau / 2.0f;
typedef float (*wave_func)(float);

enum MIDI_Action{
    NoteOn = 144,
    NoteOff = 128
};

inline float midi2hz(unsigned char note){
    float a = (note - 69) / 12.0f;
    return 440.0f * powf(2.0f, a);
}

inline float hz2dphase(float hz){
    return 2.0f * hz / float(sample_rate);
}
