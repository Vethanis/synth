#pragma once

#define min(a, b)((a) < (b) ? (a) : (b))
#define max(a, b)((a) > (b) ? (a) : (b))
#define clamp(x, a, b)(min(max(x, a), b))

struct quadratic_bezier{
    float p0, p1, p2;
    inline float value_at(float t){
        float t0 = (1.0f - t) * (1.0f - t) * p0;
        float t1 = 2.0f * (1.0f - t) * t * p1;
        float t2 = t * t * p2;
        return t0 + t1 + t2;
    }
};

template<int num_states>
struct envelope{
    float durations[num_states];
    quadratic_bezier curves[num_states];
    inline void set_state(int state, float duration, const quadratic_bezier& bez){
        durations[state] = duration;
        curves[state] = bez;
    }
    inline float value(float atime){
        int state = 0;
        for(; state < num_states - 1; state++){
            if(atime < durations[state]){
                break;
            }
            atime -= durations[state];
        }
        float normalized_time = clamp(atime / durations[state], 0.0f, 1.0f);
        return curves[state].value_at(normalized_time);
    }
};