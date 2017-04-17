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

struct midiud{
    unsigned char note;
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
            ud->note = note;
        }
        else if(action == NoteOff){
        }
    }
}

struct audioud{
    float left, right;
    unsigned char* note;
};

static int on_audio(const void* inbuf, void* outbuf, unsigned long num_frames, 
    const PaStreamCallbackTimeInfo* timeinfo, PaStreamCallbackFlags flags, void* userdata){
    
    float* output = (float*)outbuf;
    audioud* phase = (audioud*)userdata;
    float& left = phase->left;
    float& right = phase->right;
    float dphase = hz2dphase(midi2hz(phase->note[0]));
    for(unsigned i = 0; i < num_frames; i++){
        *output = left;
        output++;
        *output = right;
        output++;

        left += dphase;
        right += dphase;
        if(left > 1.0f)
            left -= 2.0f;
        if(right > 1.0f)
            right -= 2.0f;
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

    midiud midiuserdata;
    midiuserdata.note = 40;
    midiin->setCallback(on_message, &midiuserdata);

    auto err = Pa_Initialize();
    tcpa(err)

    PaStream* stream = nullptr; 
    audioud phase = {0.0f, 0.0f, &midiuserdata.note };
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