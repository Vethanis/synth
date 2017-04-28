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

#include "synth.h"

#define tcm(x){ try{ x } catch(RtMidiError& e){ puts("Had an exception:"); e.printMessage(); exit(1); } };
#define tcpa(err){ if(err != paNoError){ puts(Pa_GetErrorText(err)); exit(1); } }

using namespace std;

bool run = true;
static void on_sigint(int signal){
    run = false;
}

static void on_message(double timestamp, vector<unsigned char>* pmessage, void* userdata){
    if(!pmessage)
        return;
    auto& message = *pmessage;
    Synth* synth = (Synth*)userdata;
    if(message.size() == 3){
        auto& action = message[0];
        auto& note = message[1];
        auto& velocity = message[2];
        if(action == NoteOn){
            synth->onNoteOn(note, velocity);
        }
        else if(action == NoteOff){
            synth->onNoteOff(note);
        }
    }
}

static int on_audio(const void* inbuf, void* outbuf, unsigned long num_frames, 
    const PaStreamCallbackTimeInfo* timeinfo, PaStreamCallbackFlags flags, void* userdata){
    
    float* output = (float*)outbuf;
    Synth* synth = (Synth*)userdata;
    for(unsigned i = 0; i < num_frames; i++){
        synth->onTick(output);
        output += 2;
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

    Synth synth;

    midiin->setCallback(on_message, &synth);

    auto err = Pa_Initialize();
    tcpa(err)

    PaStream* stream = nullptr; 
    err = Pa_OpenDefaultStream(&stream, 0, 2, paFloat32, sample_rate, 32, on_audio, &synth);
    tcpa(err)

    err = Pa_StartStream(stream);
    tcpa(err)

    int num_input = 0;

    while(run){
        scanf("%i", &num_input);
        if (num_input == 0){
            run = false;
        }
        synth.onInput(num_input);
    }

    err = Pa_StopStream(stream);
    tcpa(err)

    err = Pa_Terminate();
    tcpa(err)

    return 0;
}