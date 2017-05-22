#pragma once

#include "voice.h"

// common parameters used across voices
struct voice_params{
    env_params env;
    wave_func func = saw_wave, mod_func = saw_wave;
    float unison_variance=0.01f,
        modulator_ratio=0.5f,
        modulator_amt=0.003f,
        volume=0.75f;
};

struct Voice{
    osc::MinimalOscillator osc[16];
    osc::MinimalOscillator mod[1];
    adsr env;
    unsigned char active_note;
    inline void onNoteOn(unsigned char note, unsigned char velocity, const voice_params& par){
        active_note = note;
        env.onNoteOn();
        osc::setNote(osc, note, par.unison_variance);
        osc::setNote(mod, note, par.unison_variance);
        for(auto& i : mod)
            i.dphase *= par.modulator_ratio;
    }
    inline void onNoteOff(){
        active_note = 0;
        env.onNoteOff();
    }
    inline void step(const voice_params& par){
        osc::step(osc);
        osc::step(mod);
        env.step(par.env);
    }
    inline float sample(const voice_params& par){
        osc::phaseModulate(osc, osc::sample(mod, par.mod_func) * par.modulator_amt);
        return osc::sample(osc, par.func) / 16 * env.value(par.env);
    }
};

// must be power of two!
constexpr int num_voices = 8;

struct Synth{
    Voice voices[num_voices];
    int tail=0;
    voice_params params;
    inline void onNoteOn(unsigned char note, unsigned char velocity){
        for(auto& voice : voices){
            if(voice.env.state == 2 && voice.env.cur_time >= params.env.durations[2]){
                voice.onNoteOn(note, velocity, params);
                return;
            }
        }
        for(auto& voice : voices){
            if(voice.env.state == 2 && voice.env.cur_time >= 0.5f * params.env.durations[2]){
                voice.onNoteOn(note, velocity, params);
                return;
            }
        }
        for(auto& voice : voices){
            if(voice.env.state == 2 && voice.env.cur_time >= 0.25f * params.env.durations[2]){
                voice.onNoteOn(note, velocity, params);
                return;
            }
        }
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
        for(auto& i : voices){
            value += i.sample(params);
            i.step(params);
        }
        value *= params.volume;
        buf[0] = value;
        buf[1] = -value;

    }
};