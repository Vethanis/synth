#pragma once

#include "voice.h"
#include "biquad.h"

struct voice_params
{
    biquad_filter_params filter;
    env_params env;
    env_params filter_env;
    wave_func func = saw_wave;
    wave_func mod_func = saw_wave;
    float unison_variance=0.005f;
    float modulator_ratio=0.5f;
    float modulator_amt=0.005f;
    float volume=0.75f;
};

struct Voice
{
    osc::Oscillator osc[16];
    osc::Oscillator mod[1];
    envelope env;
    envelope filter_env;
    biquad_filter filter;
    float duration = 0.0f;
    unsigned char active_note = 0;

    void onNoteOn(unsigned char note, unsigned char velocity, const voice_params& par)
    {
        duration = 0.0f;
        active_note = note;
        env.onNoteOn();
        filter_env.onNoteOn();
        osc::setNote(osc, note, par.unison_variance);
        osc::setNote(mod, note, 0.0f);
        for(auto& i : mod)
            i.dphase *= par.modulator_ratio;
    }
    void onNoteOff()
    {
        active_note = 0;
        env.onNoteOff();
        filter_env.onNoteOff();
    }
    void onTick(const voice_params& par)
    {
        duration += inv_sample_rate;
        const float mod_value = osc::multiSample(mod, par.mod_func, 16);
        osc::phaseModulate(osc, mod_value * par.modulator_amt);
        const float osc_value = osc::multiSample(osc, par.func, 16) / float(count(osc));
        env.onTick(par.env);
        filter_env.onTick(par.filter_env);
        filter.onTick(par.filter, filter_env.sample(par.filter_env), osc_value);
    }
    float sample(const voice_params& par) const 
    {
        return filter.sample() * env.sample(par.env);
    }
};

constexpr int num_voices = 8;

struct Synth
{
    Voice voices[num_voices];
    voice_params params;

    void onNoteOn(unsigned char note, unsigned char velocity)
    {
        Voice* pSelection = &voices[0];
        for(Voice& voice : voices)
        {
            if(voice.duration > pSelection->duration)
            {
                pSelection = &voice;
            }
        }
        pSelection->onNoteOn(note, velocity, params);
    }
    void onNoteOff(unsigned char note)
    {
        for(auto& i : voices)
        {
            if(i.active_note == note)
                i.onNoteOff();
        }
    }
    void onTick()
    {
        for(auto& i : voices)
        {
            i.onTick(params);
        }
    }
    void sample(float* buff) const
    {
        float value = 0.0f;
        for(const auto& i : voices)
        {
            value += i.sample(params);
        }
        value *= params.volume;
        if(value > 0.0f)
        {
            value = value / (value + 1.0f);
        }
        else
        {
            value = value / (-value + 1.0f);
        }
        buff[0] = value;
        buff[1] = -value;
    }
};