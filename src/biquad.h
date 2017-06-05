#pragma once

#include "constants.h"

enum biquad_filter_state {
    BQ_LOWPASS  = 0,
    BQ_HIPASS   = 1,
    BQ_BANDPASS = 2,
    BQ_NOTCH    = 3
};

struct biquad_filter_params{
    float F, Q;
    float env_amt;
    int state;
    biquad_filter_params(){
        F = 1000.0f;
        Q = 0.5f;
        env_amt = 100.0f;
        state = BQ_LOWPASS;
    }
    inline float getCutoff(float env_value)const{
        return F + env_amt * env_value;
    }
};

struct biquad_filter{
    float a[3];
    float b[2];
    float z[2];
    float output;
    biquad_filter(){
        memset(this, 0, sizeof(biquad_filter));
        a[0] = 1.0f;
    }
    inline void step(float input){
        output = input * a[0] + z[0];
        z[0] = input * a[1] + z[1] - b[0] * output;
        z[1] = input * a[2] - b[1] * output;
    }
    inline void update(const biquad_filter_params& params, float env_value){
        const float K = tanf(pi * (params.getCutoff(env_value) / float(sample_rate)) );
        const float KdQ = K / params.Q;
        const float KK = K*K;
        const float N = 1.0f / (1.0f + KdQ + KK);
        b[0] = 2.0f * (KK - 1.0f) * N;
        b[1] = (1.0f - KdQ + KK) * N;

        switch(params.state){
            case BQ_LOWPASS:
                a[0] = KK * N;
                a[1] = 2.0f * a[0];
                a[2] = a[0];
            break;
            
            case BQ_HIPASS:
                a[0] = N;
                a[1] = -2.0f * N;
                a[2] = N;
            break;

            case BQ_BANDPASS:
                a[0] = KdQ * N;
                a[1] = 0.0f;
                a[2] = -a[0];
            break;

            case BQ_NOTCH:
                a[0] = (1.0f + KK) * N;
                a[1] = b[0];
                a[2] = a[0];
            break;
            default:
            break;
        };
    }
};