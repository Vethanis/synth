#pragma once

#include "envelope.h"
#include "constants.h"
#include "wave.h"

namespace osc 
{
    struct Oscillator
    {
        float phase;
        float dphase;
        Oscillator() : phase(randf() * tau), dphase(hz2dphase(440.0f)) {}
    };
    // functions for dealing with a collection of things with phase and dphase
    template<typename T>
    inline void setNote(T& oscs, unsigned char note, float variance)
    {
        float dphase = hz2dphase(midi2hz(note));
        for(auto& i : oscs){
            i.dphase = dphase * (1.0f + variance * (randf() * 2.0f - 1.0f));
        }
    }
    template<typename T>
    inline void setHz(T& oscs, float hz)
    {
        float dphase = hz2dphase(hz);
        for(auto& i : oscs)
            i.dphase = dphase;
    }
    template<typename T>
    inline void step(T& oscs)
    {
        for(auto& i : oscs)
            i.phase = fmod(i.phase + i.dphase, tau);
    }
    template<typename T>
    inline float sample(T& oscs, float wt)
    {
        float value = 0.0f;
        for(auto& i : oscs)
            value += meta_wave(wt, i.phase);

        return value;
    }
    template<typename T>
    inline float multiSample(T& oscs, float wt, int sample_count)
    {
        float value = 0.0f;
        for(auto& osc : oscs)
        {
            const float dphase = osc.dphase / float(sample_count);
            for(int i = 0; i < sample_count; ++i)
            {
                osc.phase = fmod(osc.phase + dphase, tau);
                value += meta_wave(wt, osc.phase);
            }
        }
        return value / float(sample_count);
    }
    template<typename T>
    inline void phaseModulate(T& oscs, float amt)
    {        
        for(auto& i : oscs)
            i.phase += amt;
    }
};
