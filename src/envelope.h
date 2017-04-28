#pragma once

#include "constants.h"
#include <vector>

inline float quadratic_bezier(float a, float b, float t){
    float p1 = min(a, b);
    float t0 = (1.0f - t) * (1.0f - t) * a;
    float t1 = 2.0f * (1.0f - t) * t * p1;
    float t2 = t * t * b;
    return t0 + t1 + t2;
}

struct adsr{
    float values[4] = {0.0f, 1.0f, 0.75f, 0.0f};
    float durations[3] = {0.01f, 0.3f, 3.0f};
    int cur_state;
    float cur_time;
    adsr(){
        cur_state = 2;
        cur_time = 999.0f;
    }
    inline void onTick(){
        cur_time += inv_sample_rate;
        // if in attack state and past attack duration
        if(cur_state == 0 && cur_time > durations[0]){
            cur_state = 1;
            cur_time = 0.0f;
        }
    }
    inline void onNoteOn(){
        cur_state = 0;
        cur_time = 0.0f;
    }
    inline void onNoteOff(){
        cur_state = 2;
        cur_time = 0.0f;
    }
    inline float value(){
        float ntime = clamp(cur_time / durations[cur_state], 0.0f, 1.0f);
        return quadratic_bezier(values[cur_state], values[cur_state+1], ntime);
    }
};
