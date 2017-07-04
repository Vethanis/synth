#define _CRT_SECURE_NO_WARNINGS

#include <cstdio>
#include <csignal>
#include <thread>
#include "ints.h"

#include "RtAudio.h"
#include "socks.h"

using namespace std;

constexpr u16 bufferLen = 512;
static_assert((bufferLen & (bufferLen - 1)) == 0);
u16 localTime = 0;
bool isUploader = false;
udp_socket sock;
RtAudio adac;

struct Packets{

    struct Packet{
        u16 data[bufferLen];
        u16 timestamp;
    };

    static constexpr u16 num_packets = 16;
    Packet data[num_packets];
    u16 idx;

    u16* current(u16& idxOut){
        idx &= (num_packets - 1);
        data[idx].timestamp = localTime;
        idxOut = idx;
        return data[idx++].data;
    }
    void send(u16 idxIn){
        sock.send(&data[idxIn], sizeof(Packet));
    }
    void recv(u16 idxIn){
        sock.recv(&data[idxIn], sizeof(Packet));
    }
};

Packets s_packets;

s32 audioCB(void* output, void* input, u32 numFrames, double streamTime, RtAudioStreamStatus status, void* data){
    u16 idx;
    if(isUploader){
        memcpy(s_packets.current(idx), input, numFrames * sizeof(u16));
        s_packets.send(idx);
        localTime += numFrames;
    }
    else{
        memcpy(output, s_packets.current(idx), numFrames * sizeof(u16));
        s_packets.recv(idx);
        localTime += numFrames;
    }

    return 0;
}

void signal_handler(int signal){
    adac.closeStream();
    exit(0);
}

int main(int argc, char** argv){
    if(argc != 4){
        puts("Usage: program <ipv4 address> <port> <u/d>");
        return 1;
    }
    char inputString[256] = {0};

    if(adac.getDeviceCount() < 1){
        puts("No audio devices found :(");
        return 1;
    }

    // Network configuration

    s32 sockStatus = 0;
    isUploader = strstr(argv[3], "u") != nullptr;
    sockStatus = sock.connect(argv[1], atoi(argv[2]), !isUploader);
    puts(sock.status_string(sockStatus));
    if(sockStatus != SOCK_OK){
        return 1;
    }

    // Audio configuration

    signal(SIGINT, signal_handler);

    const u32 numDevices = adac.getDeviceCount();
    constexpr u32 sample_rate = 44100;
    u32 buffBytes, buffFrames = bufferLen;
    RtAudio::StreamParameters inParams, outParams;

    RtAudio::DeviceInfo dInfo;
    for(u32 i = 0; i < numDevices; i++){
        dInfo = adac.getDeviceInfo(i);
        
        if(dInfo.probed){
            if(dInfo.isDefaultInput){
                inParams.deviceId = i;
            }
            if(dInfo.isDefaultOutput){
                outParams.deviceId = i;
            }
        }
    }

    inParams.nChannels = adac.getDeviceInfo(inParams.deviceId).inputChannels;
    outParams.nChannels = adac.getDeviceInfo(outParams.deviceId).outputChannels;

    try{
        adac.openStream(&outParams, &inParams, RTAUDIO_SINT16, sample_rate, &buffFrames, &audioCB, nullptr);
    }
    catch(RtAudioError& e){
        puts("Open stream error: ");
        e.printMessage();
        return 1;
    }

    // Audio stream begin

    buffBytes = buffFrames * outParams.nChannels * sizeof(u16);

    try{
        adac.startStream();
    }
    catch(RtAudioError& e){
        puts("Start stream error: ");
        e.printMessage();
        if(adac.isStreamOpen()){
            adac.closeStream();
        }
    }

    while(adac.isStreamOpen()){
        std::this_thread::sleep_for(10ms);
    };

    return 0;
}