#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>
#include <csignal>
#include <ctime>
#include <random>

#include "RtMidi.h"
#include "portaudio.h"

#include "synth.h"
#include "window.h"
#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"

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
    RtMidiIn* midiin;
    tcm( midiin = new RtMidiIn(); )

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
    tcpa(err);

    PaStream* stream = nullptr; 
    err = Pa_OpenDefaultStream(&stream, 0, 2, paFloat32, sample_rate, 16, on_audio, &synth);
    tcpa(err);

    err = Pa_StartStream(stream);
    tcpa(err);

    auto* window = window::init(800, 600, 3, 3, "Synth");

    assert(ImGui_ImplGlfwGL3_Init(window, true));
    int winflags = ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove;
    int iwave = 2, imodwave = 1, imodratio = 0;
    wave_func waves[] = { sine_wave, triangle_wave, square_wave, saw_wave };
    while(window::is_open(window)){
        ImGui_ImplGlfwGL3_NewFrame();
        ImGui::SetNextWindowPosCenter();
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin("Synth controls", nullptr, winflags);
        ImGui::SliderFloat("Volume", &synth.params.volume, 0.0f, 1.0f, nullptr, 2.0f);
        ImGui::SliderInt("Wave", &iwave, 0, 3);
        ImGui::SliderInt("Modulator Wave", &imodwave, 0, 3);
        ImGui::SliderFloat("Unison Variance", &synth.params.unison_variance, 0.0f, 0.1f, "%0.5f", 2.0f);
        ImGui::SliderFloat("Attack", &synth.params.env.durations[0], 0.01f, 2.0f, nullptr, 2.0f);
        ImGui::SliderFloat("Decay", &synth.params.env.durations[1], 0.01f, 5.0f, nullptr, 2.0f);
        ImGui::SliderFloat("Sustain", &synth.params.env.values[2], 0.0f, 1.0f, nullptr, 2.0f);
        ImGui::SliderFloat("Release", &synth.params.env.durations[2], 0.01f, 5.0f, nullptr, 2.0f);
        ImGui::SliderFloat("Modulation Amount", &synth.params.modulator_amt, 0.0f, 0.25f, "%0.5f", 2.0f);
        ImGui::SliderInt("Modulator Ratio", &imodratio, 0, 20);
        synth.params.func = waves[iwave];
        synth.params.mod_func = waves[imodwave];
        float modratio = 1.0f;
        for(int i = 10; i > imodratio; i--)
            modratio *= 0.5f;
        for(int i = 10; i < imodratio; i++)
            modratio *= 2.0f;
        synth.params.modulator_ratio = modratio;

        ImGui::End();
        ImGui::Render();
        window::swap(window);
    }

    ImGui_ImplGlfwGL3_Shutdown();

    err = Pa_StopStream(stream);
    tcpa(err);

    err = Pa_Terminate();
    tcpa(err);

    delete midiin;

    return 0;
}