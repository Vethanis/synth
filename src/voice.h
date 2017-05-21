#pragma once

#include "envelope.h"
#include "constants.h"
#include "wave.h"

struct Oscillator{
    float phase, dphase, value;
    wave_func func;
    Oscillator() : phase(randf() * tau), dphase(randf()), func(&saw_wave){
        value = func(phase);
    }
    inline void setNote(unsigned char note){
        dphase = hz2dphase(midi2hz(note));
    }
    inline void setHz(float hz){
        dphase = hz2dphase(hz);
    }
    inline void step(){
        phase += fmod(phase + dphase, tau);
        value = func(phase);
    }
};

namespace osc {
    struct MinimalOscillator{
        float phase, dphase;
        MinimalOscillator() : phase(randf() * tau), dphase(randf()){
        }
    };
    // functions for dealing with a collection of things with phase and dphase
    template<typename T>
    inline void setNote(T& oscs, unsigned char note, float variance){
        float dphase = hz2dphase(midi2hz(note));
        for(auto& i : oscs){
            i.dphase = dphase * (1.0f + variance * (randf() * 2.0f - 1.0f));
        }
    }
    template<typename T>
    inline void setHz(T& oscs, float hz){
        float dphase = hz2dphase(hz);
        for(auto& i : oscs)
            i.dphase = dphase;
    }
    template<typename T>
    inline void step(T& oscs){
        for(auto& i : oscs)
            i.phase = fmod(i.phase + i.dphase, tau);
    }
    template<typename T>
    inline float sample(T& oscs, wave_func func){
        float value = 0.0f;
        for(auto& i : oscs)
            value += func(i.phase);
        return value;
    }
    template<typename T>
    inline void phaseModulate(T& oscs, float amt){        
        for(auto& i : oscs)
            i.phase += amt;
    }
};

// 18KB object; don't put many of these on the stack
struct KarplusOscillator{
    float buffer[sample_rate / 10];
    const int buffer_len = sample_rate / 10;
    int num_samples, cur_sample;
    KarplusOscillator(){
        cur_sample = 0;
        num_samples = buffer_len;
    }
    inline void onNote(unsigned char note){
        cur_sample = 0;
        num_samples = min(int( sample_rate / midi2hz(note) ), buffer_len - 2);
        for(int i = 0; i < buffer_len; i++){
            buffer[i] = randf() * 2.0f - 1.0f;
        }
    }
    inline float onTick(){
        int next_sample = cur_sample + 1;
        if(next_sample >= int(num_samples + randf() * 2.0f) )
            next_sample = 0;
        buffer[cur_sample] = (buffer[next_sample] + buffer[cur_sample]) * 0.5f;
        cur_sample = next_sample;
        return buffer[cur_sample];
    }
};
