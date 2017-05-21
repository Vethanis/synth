#pragma once

#include "envelope.h"
#include "constants.h"
#include "wave.h"


struct Oscillator{
    float phase, dphase;
    wave_func func;
    Oscillator() : phase(randf() * tau), dphase(randf()), func(&saw_wave){
    }
    inline void setNote(unsigned char anote){
        dphase = hz2dphase(midi2hz(anote));
    }
    inline void setHz(float hz){
        dphase = hz2dphase(hz);
    }
    inline float onTick(){
        phase = fmod(phase + dphase, tau);
        return func(phase);
    }
    inline void phaseModulate(Oscillator& modulator, float amt){
        phase += amt * modulator.func(modulator.phase);
    }
};

template<int num_voices>
struct MultiOscillator{
    Oscillator oscs[num_voices];
    inline void setNote(unsigned char note, float variance){
        float hz = midi2hz(note);
        for(auto& i : oscs){
            i.setHz( hz );
            i.dphase *= 1.0f + variance * (randf() * 2.0f - 1.0f);
        }
    }
    inline float onTick(){
        float value = 0.0f;
        for(auto& i : oscs)
            value += i.onTick();
        return value / num_voices;
    }
    inline void phaseModulate(Oscillator& modulator, float amt){        
        for(auto& i : oscs)
            i.phaseModulate(modulator, amt);
    }
    inline void setWave(wave_func afunc){
        for(auto& i : oscs)
            i.func = afunc;
    }
};

struct KarplusOscillator{
    float* buffer;
    int buffer_len, num_samples, cur_sample;
    KarplusOscillator(){
        buffer_len = sample_rate / 10;
        buffer = new float[buffer_len];
        cur_sample = 0;
        num_samples = buffer_len;
    }
    ~KarplusOscillator(){
        delete[] buffer;
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
