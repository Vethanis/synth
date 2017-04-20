#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <memory>
#include <csignal>
#include <vector>
#include <thread>

#include "RtMidi.h"
#include "portaudio.h"

#include "envelope.h"

#define tcm(x){ try{ x } catch(RtMidiError& e){ puts("Had an exception:"); e.printMessage(); exit(1); } };
#define tcpa(err){ if(err != paNoError){ puts(Pa_GetErrorText(err)); exit(1); } }

using namespace std;

constexpr int sample_rate = 44100;
constexpr float tau = 6.2831853f;
constexpr float pi = tau / 2.0f;

enum MIDI_Action{
    NoteOn = 144,
    NoteOff = 128
};

inline float midi2hz(unsigned char note){
    float a = (note - 69) / 12.0f;
    return 440.0f * powf(2.0f, a);
}

inline float hz2dphase(float hz){
    return 2.0f * hz / float(sample_rate);
}

bool run = true;
static void on_sigint(int signal){
    run = false;
}

inline float lerp(float a, float b, float alpha){
    return (1.0f - alpha) * a + alpha * b;
}
inline float fract(float f){
    return f - float(int(f));
}
// whiteish noise
inline float randf() {
    static int f = 41020057;
    f = (f ^ 61) ^ (f >> 16);
    f *= 9;
    f = f ^ (f >> 4);
    f *= 0x27d4eb2d;
    f = f ^ (f >> 15);
    return fract(float(f) * 2.3283064e-10f);
}
// bluish noise
inline float brandf(){
    float a = randf() * 0.5f;
    float b = 0.5f + randf() * 0.5f;
    return (a + b) * 0.5f;
}
// pinkish noise
inline float prandf(){
    return (randf() + randf()) * 0.5f;
}

float saw_wave(float phase){
    phase /= tau;
    return lerp(-1.0f, 1.0f, phase);
}
float sine_wave(float phase){
    return sinf(phase);
}
float square_wave(float phase){
    return phase < pi ? 1.0f : -1.0f;
}
float triangle_wave(float phase){
    if(phase < pi){
        return lerp(-1.0f, 1.0f, phase / pi);
    }
    else{
        phase -= pi;
        return lerp(1.0f, -1.0f, phase / pi);
    }
}

typedef float (*wave_func)(float);

struct Voice{
    float phase, dphase, env_time;
    Voice() : phase(0.0f), dphase(0.0f), env_time(120.0f){
    }
};

struct Voices{
    Voice voices[8];
    wave_func func;
    envelope<2> env;
    int tail;
    float left, right, g_volume;
    Voices() : func(&saw_wave), tail(0), g_volume(0.125f){
        env.set_state(0, 0.5f, {0.0f, 0.0f, 1.0f});
        env.set_state(1, 5.0f, {1.0f, 0.0f, 0.0f});
    }
    inline void onnote(unsigned char anote){
        voices[tail].dphase = hz2dphase(midi2hz(anote));
        voices[tail].env_time = 0.0f;
        tail = (tail+1) & 7;
    }
    inline void ontick(){
        left = 0.0f;
        right = 0.0f;
        for(int i = 0; i < 8; i++){
            Voice& v = voices[i];
            const float sample = func(fmod(v.phase + brandf() * v.dphase, tau));
            const float env_val = env.value(v.env_time);
            const float val = sample * env_val * g_volume;
            if(brandf() > 0.5f){
                left += val;
                right -= val;
            }
            else{
                left -= val;
                right += val;
            }
            v.phase = fmod(v.phase + v.dphase, tau);
            v.env_time += 1.0f / sample_rate;
        }
    }
};

static void on_message(double timestamp, vector<unsigned char>* pmessage, void* userdata){
    if(!pmessage)
        return;
    auto& message = *pmessage;
    Voices* voice = (Voices*)userdata;
    if(message.size() == 3){
        auto& action = message[0];
        auto& note = message[1];
        auto& velocity = message[2];
        if(action == NoteOn){
            voice->onnote(note);
        }
        else if(action == NoteOff){
        }
    }
}


static int on_audio(const void* inbuf, void* outbuf, unsigned long num_frames, 
    const PaStreamCallbackTimeInfo* timeinfo, PaStreamCallbackFlags flags, void* userdata){
    
    float* output = (float*)outbuf;
    Voices* vcs = (Voices*)userdata;
    for(unsigned i = 0; i < num_frames; i++){
        *output = vcs->left;
        output++;
        *output = vcs->right;
        output++;
        vcs->ontick();
    }

    return 0;
}

int main(){
    srand((unsigned)time(0));
    unique_ptr<RtMidiIn> midiin;
    tcm( midiin = make_unique<RtMidiIn>(); )

    if(midiin->getPortCount() < 1){
        puts("No ports open, quitting.");
        exit(1);
    }
    puts(midiin->getPortName(0).c_str());

    midiin->openPort(0);
    signal(SIGINT, on_sigint);

    Voices phase;

    midiin->setCallback(on_message, &phase);

    auto err = Pa_Initialize();
    tcpa(err)

    PaStream* stream = nullptr; 
    err = Pa_OpenDefaultStream(&stream, 0, 2, paFloat32, sample_rate, 32, on_audio, &phase);
    tcpa(err)

    err = Pa_StartStream(stream);
    tcpa(err)

    int num_input = 0;

    while(run){
        scanf("%i", &num_input);
        switch(num_input){
            case 0:
                run = false;
                break;
            case 1:
                phase.func = &sine_wave;
                break;
            case 2:
                phase.func = &triangle_wave;
                break;
            case 3:
                phase.func = &square_wave;
                break;
            default:
            case 4:
                phase.func = &saw_wave;
        }
    }

    err = Pa_StopStream(stream);
    tcpa(err)

    err = Pa_Terminate();
    tcpa(err)

    return 0;
}