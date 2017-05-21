#pragma once

#include "voice.h"

// common parameters used across voices
struct voice_params{
    env_params env;
    float unison_variance=0.01f,
        modulator_ratio=0.5f,
        modulator_amt=0.003f,
        volume=0.75f;
};

struct Voice{
    MultiOscillator<16> osc;
    Oscillator modulator;
    adsr env;
    unsigned char active_note;
    inline void onNoteOn(unsigned char note, unsigned char velocity, const voice_params& par){
        active_note = note;
        env.onNoteOn();
        osc.setNote(note, par.unison_variance);
        modulator.setNote(note);
        modulator.dphase *= par.modulator_ratio;
    }
    inline void onNoteOff(){
        active_note = 0;
        env.onNoteOff();
    }
    inline float onTick(const voice_params& par){
        env.onTick(par.env);
        osc.phaseModulate(modulator, par.modulator_amt);
        modulator.onTick();
        return osc.onTick() * env.value(par.env);
    }
};

// must be power of two!
constexpr int num_voices = 8;

struct Synth{
    Voice voices[num_voices];
    int tail=0;
    voice_params params;
    inline void onNoteOn(unsigned char note, unsigned char velocity){
        voices[tail].onNoteOn(note, velocity, params);
        tail = (tail + 1) & (num_voices - 1);
    }
    inline void onNoteOff(unsigned char note){
        for(auto& i : voices){
            if(i.active_note == note)
                i.onNoteOff();
        }
    }
    inline void onTick(float* buf){
        float value = 0.0f;
        for(auto& i : voices)
            value += i.onTick(params);
        value *= params.volume;
        buf[0] = value;
        buf[1] = -value;
    }
    inline void setWave(wave_func func){
        for(auto& i : voices)
            i.osc.setWave(func);
    }
    inline void setModulatorWave(wave_func func){
        for(auto& i : voices)
            i.modulator.func = func;
    }
};