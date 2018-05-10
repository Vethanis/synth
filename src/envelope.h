#pragma once

#include "constants.h"

struct env_params
{
    float attack = 0.01f;
    float decay = 1.9f;
    float attack_power = 2.0f;
    float decay_power = 2.0f;
    bool sustain = true;
};

struct envelope
{
    float cur_time = 0.0f;
    float gate_off_value = 0.0f;
    float value = 0.0f;
    bool gate = false;

    void onNoteOn()
    {
        cur_time = 0.0f;
        gate = true;
    }
    void onNoteOff()
    {
        gate_off_value = value;
        cur_time = 0.0f;
        gate = false;
    }
    void onTick(const env_params& p)
    {
        cur_time += inv_sample_rate;      

        if(!p.sustain && gate && cur_time > p.attack)
        {
            onNoteOff();
        }

        if(gate)
        {
            value = clamp(cur_time / p.attack, 0.0f, 1.0f);
        }
        else
        {
            const float alpha = clamp(cur_time / p.decay, 0.0f, 1.0f);
            value = lerp(gate_off_value, 0.0f, alpha);
        }
    }
    float sample(const env_params& p) const 
    {
        if(gate)
            return powf(value, p.attack_power);

        return powf(value, p.decay_power);
    }
};
