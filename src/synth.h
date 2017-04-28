#pragma once

#include "voice.h"

struct Voice{
    MultiOscillator<16> osc;
    Oscillator modulator;
    adsr env;
    unsigned char active_note;
    inline void onNoteOn(unsigned char note, unsigned char velocity){
        active_note = note;
        env.onNoteOn();
        osc.setNote(note, 0.0001f);
        modulator.setNote(note);
        modulator.dphase *= 0.5f;
    }
    inline void onNoteOff(){
        active_note = 0;
        env.onNoteOff();
    }
    inline float onTick(){
        env.onTick();
        osc.phaseModulate(modulator, 0.01f);
        modulator.onTick();
        return osc.onTick() * env.value();
    }
    inline void onInput(int input){
        osc.onInput(input);
        modulator.onInput(input);
    }
};

struct Synth{
    Voice voices[8];
    int tail;
    Synth() : tail(0){
    }
    inline void setEnv(const adsr& env){
        for(int i = 0; i < 8; i++){
            voices[i].env = env;
        }
    }
    inline void onNoteOn(unsigned char note, unsigned char velocity){
        auto& voice = voices[tail];
        voice.onNoteOn(note, velocity);
        tail = (tail + 1) & 7;
    }
    inline void onNoteOff(unsigned char note){
        for(int i = 0; i < 8; i++){
            auto& voice = voices[i];
            if(voice.active_note == note){
                voice.onNoteOff();
            }
        }
    }
    inline void onTick(float* buf){
        float value = 0.0f;
        for(int i = 0; i < 8; i++){
            value += voices[i].onTick();
        }
        float* left = buf;
        float* right = buf+1;
        *left = value;
        *right = -*left;
    }
    inline void onInput(int input){
        for(int i = 0; i < 8; i++){
            voices[i].onInput(input);
        }
    }
};