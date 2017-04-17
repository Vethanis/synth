#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <memory>
#include <csignal>
#include <vector>
#include <thread>

#include "RtMidi.h"
#include "portaudio.h"

#define tcm(x){ try{ x } catch(RtMidiError& e){ puts("Had an exception:"); e.printMessage(); exit(1); } };
#define tcpa(err){ if(err != paNoError){ puts(Pa_GetErrorText(err)); exit(1); } }

using namespace std;

constexpr int sample_rate = 44100;

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

float saw_wave(float phase){
    phase /= 6.2831853f;
    return lerp(-1.0f, 1.0f, phase);
}
float sine_wave(float phase){
    return sinf(phase);
}
float square_wave(float phase){
    return phase < 3.141592f ? 1.0f : -1.0f;
}
float triangle_wave(float phase){
    if(phase < 3.141592f){
        return lerp(-1.0f, 1.0f, phase / 3.141592f);
    }
    else{
        phase -= 3.141592f;
        return lerp(1.0f, -1.0f, phase / 3.141592f);
    }
}

typedef float (*wave_func)(float);

struct voices{
    float phase[8];
    float dphase[8];
    float amplitude[8];
    wave_func func;
    int tail;
    float out;
    voices() : func(&saw_wave), tail(0), out(0.0f){
        for(int i = 0; i < 8; i++){
            amplitude[i] = 0.0f;
            phase[i] = 0.0f;
            dphase[i] = 0.0f;
        }
    }
    inline void onnote(unsigned char anote){
        dphase[tail] = hz2dphase(midi2hz(anote));
        amplitude[tail] = 0.5f;
        tail = (tail+1) & 7;
    }
    inline void ontick(){
        out = 0.0f;
        for(int i = 0; i < 8; i++){
            out += amplitude[i] * func(phase[i]);
            phase[i] += dphase[i];
            phase[i] = fmod(phase[i], 6.2831853f);
            amplitude[i] *= 0.9999f;
        }
    }
};

struct midiud{
    unsigned char note;
    voices* voice;
};

static void on_message(double timestamp, vector<unsigned char>* pmessage, void* userdata){
    if(!pmessage)
        return;
    auto& message = *pmessage;
    midiud* ud = (midiud*)userdata;
    if(message.size() == 3){
        auto& action = message[0];
        auto& note = message[1];
        auto& velocity = message[2];
        if(action == NoteOn){
            ud->voice->onnote(note);
        }
        else if(action == NoteOff){
        }
    }
}


static int on_audio(const void* inbuf, void* outbuf, unsigned long num_frames, 
    const PaStreamCallbackTimeInfo* timeinfo, PaStreamCallbackFlags flags, void* userdata){
    
    float* output = (float*)outbuf;
    voices* vcs = (voices*)userdata;
    for(unsigned i = 0; i < num_frames; i++){
        *output = vcs->out;
        output++;
        *output = vcs->out;
        output++;
        vcs->ontick();
    }

    return 0;
}

int main(){
    unique_ptr<RtMidiIn> midiin;
    tcm( midiin = make_unique<RtMidiIn>(); )

    if(midiin->getPortCount() < 1){
        puts("No ports open, quitting.");
        exit(1);
    }
    puts(midiin->getPortName(0).c_str());

    midiin->openPort(0);
    signal(SIGINT, on_sigint);
    voices phase;

    midiud midiuserdata;
    midiuserdata.note = 40;
    midiuserdata.voice = &phase;
    midiin->setCallback(on_message, &midiuserdata);

    auto err = Pa_Initialize();
    tcpa(err)

    PaStream* stream = nullptr; 
    err = Pa_OpenDefaultStream(&stream, 0, 2, paFloat32, sample_rate, 32, on_audio, &phase);
    tcpa(err)

    err = Pa_StartStream(stream);
    tcpa(err)

    while(run){
        this_thread::sleep_for(0.1s);
    }

    err = Pa_StopStream(stream);
    tcpa(err)

    err = Pa_Terminate();
    tcpa(err)

    return 0;
}