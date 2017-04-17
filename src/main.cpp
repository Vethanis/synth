#include <cstdio>
#include <cstdlib>
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

bool run = true;
static void on_sigint(int signal){
    run = false;
}

static void on_message(double timestamp, vector<unsigned char>* pmessage, void* userdata){
    if(!pmessage)
        return;
    auto& message = *pmessage;
    if(message.size() == 3){
        auto& action = message[0];
        auto& note = message[1]; // C3 = 60
        auto& velocity = message[2];
        if(action == NoteOn){
            printf("Note-on %i\n", note);
        }
        else if(action == NoteOff){
            printf("Note-off %i\n", note);
        }
    }
}

struct aphase{
    float left, right;
};

static int on_audio(const void* inbuf, void* outbuf, unsigned long num_frames, 
    const PaStreamCallbackTimeInfo* timeinfo, PaStreamCallbackFlags flags, void* userdata){
    
    float* output = (float*)outbuf;
    aphase* phase = (aphase*)userdata;
    float& left = phase->left;
    float& right = phase->right;
    for(unsigned i = 0; i < num_frames; i++){
        *output = left;
        output++;
        *output = right;
        output++;

        left += 0.002f;
        right += 0.004f;
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

    midiin->setCallback(on_message, nullptr);

    auto err = Pa_Initialize();
    tcpa(err)

    PaStream* stream = nullptr; 
    aphase phase = {0.0f, 0.0f};
    err = Pa_OpenDefaultStream(&stream, 0, 2, paFloat32, sample_rate, 256, on_audio, &phase);
    tcpa(err)

    err = Pa_StartStream(stream);
    tcpa(err)

    while(run){
        this_thread::sleep_for(1s);
    }

    err = Pa_StopStream(stream);
    tcpa(err)

    err = Pa_Terminate();
    tcpa(err)

    return 0;
}