#pragma once

#include "constants.h"

enum eFilterType : int
{
    BQ_LOWPASS = 0,
    BQ_HIPASS,
    BQ_BANDPASS,
    BQ_NOTCH,
    BQ_COUNT
};

struct biquad_filter_params
{
    float F = 1.0f;
    float Q = 0.5f;
    float env_amt = 4000.0f;
    eFilterType type = BQ_LOWPASS;

    float getCutoff(float env_value) const
    {
        return clamp(F + env_amt * env_value, 20.0f, 20000.0f);
    }
};

struct biquad_filter
{
    float a[3] = {1.0f, 0.0f, 0.0f};
    float b[2] = {0.0f, 0.0f};
    float z[2] = {0.0f, 0.0f};
    float output = 0.0f;

    float sample() const { return output; }
    void onTick(const biquad_filter_params& params, float env_value, float input)
    {
        const float K = tanf(pi * (params.getCutoff(env_value) / float(sample_rate)) );
        const float KdQ = K / params.Q;
        const float KK = K*K;
        const float N = 1.0f / (1.0f + KdQ + KK);

        b[0] = 2.0f * (KK - 1.0f) * N;
        b[1] = (1.0f - KdQ + KK) * N;

        switch(params.type)
        {
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
        }
        
        output = input * a[0] + z[0];
        z[0] = input * a[1] + z[1] - b[0] * output;
        z[1] = input * a[2] - b[1] * output;
    }
};