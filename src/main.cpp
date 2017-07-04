#define _CRT_SECURE_NO_WARNINGS

#include <cstdio>
#include <thread>
#include <csignal>
#include "ints.h"

#include "RtAudio.h"
#include "socks.h"

using namespace std;

constexpr s32 bufferLen = 512;
constexpr s32 numChannels = 2;
constexpr s32 num_packets = 8;
constexpr s32 packetLen = (numChannels * bufferLen) + 1;
constexpr s32 packetBytes = packetLen * sizeof(s32);
constexpr s32 bufferBytes = bufferLen * numChannels * sizeof(s32);
constexpr u32 sample_rate = 44100;
static_assert((bufferLen & (bufferLen - 1)) == 0);
bool isUploader = false;
bool netrun = true;
udp_socket sock;
RtAudio adac;

struct Packets{
    s32 ids[num_packets];
    s32 data[num_packets][packetLen];
    s32 recBuffer[packetLen];
    s32 localIdx, netIdx;
    Packets(){ memset(this, 0, sizeof(Packets)); }
    s32 wrap(s32 id){ return id & (num_packets - 1); }
    void advanceLocal(){ localIdx = wrap(localIdx+1); }
    void advanceNet(){ netIdx = wrap(netIdx+1); }
    void send(){
        data[netIdx][0] = netIdx;
        sock.send(data[netIdx], packetBytes);
        advanceNet();
    }
    void recv(){ 
        sock.recv(recBuffer, packetBytes); 
        s32 peerId = recBuffer[0];
        memcpy(data[peerId], recBuffer, packetBytes);
        if(peerId == 1){
            localIdx = 0;
        }
    }
    s32* currentBuffer(){ return &data[localIdx][1]; }
};

Packets s_packets;

s32 audioCB(void* output, void* input, u32 numFrames, double streamTime, RtAudioStreamStatus status, void* data){
    if(status)
        return 0;
    
    if(isUploader){
        memcpy(s_packets.currentBuffer(), input, bufferBytes);
        s_packets.send();
    }
    else{
        memcpy(output, s_packets.currentBuffer(), bufferBytes);
    }

    s_packets.advanceLocal();
    return 0;
}

void netCB(){
    if(!isUploader){
        while(netrun){
            s_packets.recv();
        }
    }
}

void sig_handler(int sig){
    netrun = false;
}

int main(int argc, char** argv){
    if(argc != 4){
        puts("Usage: program <ipv4 address> <port> <u/d>");
        return 1;
    }

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

    const u32 numDevices = adac.getDeviceCount();
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

    inParams.nChannels = numChannels;
    outParams.nChannels = numChannels;

    try{
        u32 buflen = bufferLen;
        adac.openStream(&outParams, &inParams, RTAUDIO_SINT32, sample_rate, &buflen, &audioCB, nullptr);
    }
    catch(RtAudioError& e){
        puts("Open stream error: ");
        e.printMessage();
        return 1;
    }

    // Audio stream begin

    try{
        adac.startStream();
    }
    catch(RtAudioError& e){
        puts("Start stream error: ");
        e.printMessage();
        return 1;
    }

    signal(0xffffffff, sig_handler);
    std::thread net_thread(netCB);
    netrun = true;

    while(netrun){
        std::this_thread::sleep_for(1ms);
    };

    return 0;
}