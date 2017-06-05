#pragma once

#include "constants.h"

struct env_params{
    float durations[3] = {0.01f, 0.3f, 4.0f};
    float values[4] = {0.0f, 1.0f, 0.85f, 0.0f};
};

struct adsr{
    int state;
    float cur_time;
    float last_value;
    float last_sustain_value;
    float output;
    adsr(){
        state = 2;
        cur_time = 60.0f;
        last_value = 0.0f;
        last_sustain_value = 0.0f;
        output = 0.0f;
    }
    inline void onNoteOn(){
        state = 0;
        cur_time = 0.0f;
        last_sustain_value = 0.0f;
    }
    inline void onNoteOff(){
        last_sustain_value = last_value;
        state = 2;
        cur_time = 0.0f;
    }
    inline void step(const env_params& p){        
        cur_time += inv_sample_rate;
        if(!state && cur_time > p.durations[state]){
            state++;
            cur_time = 0.0f;
            last_sustain_value = 1.0f;
        }
        float ntime = cur_time / p.durations[state];
        ntime = clamp(ntime, 0.0f, 1.0f);
        last_value = lerp(last_sustain_value, p.values[state+1], ntime);
        output = last_value * last_value;
    }
};
