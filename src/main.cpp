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
void on_sigint(int signal)
{
    run = false;
}

void on_message(double timestamp, vector<unsigned char>* pmessage, void* userdata)
{
    if(!pmessage)
        return;
    
    const auto& message = *pmessage;
    Synth& synth = *(Synth*)userdata;

    if(message.size() == 3)
    {
        const auto action = message[0];
        const auto note = message[1];
        const auto velocity = message[2];
        if(action == NoteOn)
        {
            synth.onNoteOn(note, velocity);
        }
        else if(action == NoteOff)
        {
            synth.onNoteOff(note);
        }
    }
}

int on_audio(const void* inbuf, void* outbuf, unsigned long num_frames, 
    const PaStreamCallbackTimeInfo* timeinfo, PaStreamCallbackFlags flags, void* userdata)
{
    float* output = (float*)outbuf;
    Synth* synth = (Synth*)userdata;
    for(unsigned i = 0; i < num_frames; i++)
    {
        synth->onTick();
        synth->sample(output);
        output += 2;
    }

    return 0;
}

int main()
{
    g_randSeed = (unsigned)time(nullptr);
    srand(g_randSeed);

    RtMidiIn* midiin;
    tcm( midiin = new RtMidiIn(); )

    if(midiin->getPortCount() < 1)
    {
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
    err = Pa_OpenDefaultStream(&stream, 0, 2, paFloat32, sample_rate, 256, on_audio, &synth);
    tcpa(err);

    err = Pa_StartStream(stream);
    tcpa(err);

    auto* window = window::init(900, 900, 3, 3, "Synth");

    assert(ImGui_ImplGlfwGL3_Init(window, true));
    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = 2.0f;

    int winflags = ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove;
    int imodratio = 9;

    while(window::is_open(window) && run)
    {
        ImGui_ImplGlfwGL3_NewFrame();
        ImGui::SetNextWindowPosCenter();
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("Synth controls", nullptr, winflags);
        ImGui::SliderFloat("Volume", &synth.params.volume, 0.0f, 2.0f, nullptr, 2.0f);

        {
            ImGui::Separator();
            ImGui::Text("Oscillator Settings");
            ImGui::SliderFloat("Wave", &synth.params.func, eTriangleWave, eNoiseWave);
            ImGui::SliderFloat("Unison Variance", &synth.params.unison_variance, 0.0f, 0.1f, "%0.5f", 2.0f);
            ImGui::SliderFloat("Modulator Wave", &synth.params.mod_func, eTriangleWave, eNoiseWave);
            ImGui::SliderFloat("Modulation Amount", &synth.params.modulator_amt, 0.0f, 0.25f, "%0.5f", 2.0f);
            ImGui::SliderInt("Modulator Ratio", &imodratio, 0, 20);
        }

        auto DisplayEnvelope = [](env_params& p)
        {
            ImGui::ID id(&p);
            ImGui::SliderFloat("Attack", &p.attack, 0.01f, 5.0f, nullptr, 2.0f);
            ImGui::SliderFloat("Decay", &p.decay, 0.01f, 5.0f, nullptr, 2.0f);
            ImGui::SliderFloat("Attack Power", &p.attack_power, 0.1f, 4.0f, nullptr, 2.0f);
            ImGui::SliderFloat("Decay Power", &p.decay_power, 0.1f, 4.0f, nullptr, 2.0f);
            ImGui::Checkbox("Sustain", &p.sustain);
        };

        {
            ImGui::Separator();
            ImGui::Text("VCA Settings");
            DisplayEnvelope(synth.params.env);
        }

        {
            ImGui::Separator();
            ImGui::Text("Filter Settings");
            ImGui::SliderFloat("Filter Cutoff", &synth.params.filter.F, 1.0f, 20000.0f, nullptr, 2.0f);
            ImGui::SliderFloat("Filter Resonance", &synth.params.filter.Q, 0.01f, 10.0f);
            ImGui::SliderInt("Filter Mode", (int*)&synth.params.filter.type, 0, BQ_COUNT - 1);
            ImGui::SliderFloat("Filter Env", &synth.params.filter.env_amt, 0.0f, 20000.0f, nullptr, 2.0f);
            ImGui::Separator();
            ImGui::Text("Filter Envelope Settings");
            DisplayEnvelope(synth.params.filter_env);
        }

        {
            float modratio = 1.0f;
            for(int i = 10; i > imodratio; i--)
                modratio *= 0.5f;
            for(int i = 10; i < imodratio; i++)
                modratio *= 2.0f;

            synth.params.modulator_ratio = modratio;
        }

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