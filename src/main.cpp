#define _CRT_SECURE_NO_WARNINGS

#include <cstdio>
#include <ctime>
#include <thread>
#include "ints.h"

#include "RtAudio.h"
#include "socks.h"

using namespace std;

constexpr u32 networkBufferLen = 512;
static_assert((networkBufferLen & (networkBufferLen - 1)) == 0);
float networkBuffer[networkBufferLen] = {0};
udp_socket sock;
u32 networkIndex = 0;
bool isUploading = false;
bool netrun = true;
bool needNetworkUpdate = false;

s32 audioCB(void* output, void* input, u32 numFrames, double streamTime, RtAudioStreamStatus status, void* data){
    if(status){
        puts("Stream overflow detected");
        return 0;
    }

    u64* bytes = (u64*)data;
    u32 numFloats = u32((*bytes) / sizeof(float));
    float* inBuf = (float*)input;
    float* outBuf = (float*)output;

    if(isUploading){
        for(u32 bufIdx = 0; bufIdx < numFloats; bufIdx++){
            networkBuffer[networkIndex++] = inBuf[bufIdx];
            networkIndex &= (networkBufferLen - 1);
        }
        needNetworkUpdate = true;
    }
    else{
        for(u32 bufIdx = 0; bufIdx < numFloats; bufIdx++){
            outBuf[bufIdx] = networkBuffer[networkIndex++];
            networkIndex &= (networkBufferLen - 1);
        }
        needNetworkUpdate = true;
    }

    return 0;
}

void networkCB(){
    u32 len = networkBufferLen * sizeof(float);

    if(isUploading){
        while(netrun){
            if(needNetworkUpdate){
                s32 numBytes = sock.send(&networkBuffer[0], len);
                needNetworkUpdate = false;
                if(numBytes != len){
                    printf("Sent %d bytes\n", numBytes);
                }
            }
        }
    }
    else{
        while(netrun){
            if(needNetworkUpdate){
                s32 numBytes = sock.recv(&networkBuffer[0], len);
                needNetworkUpdate = false;
                if(numBytes != len){
                    printf("Received %d bytes\n", numBytes);
                }
            }
        }
    }
}

int main(){
    char networkRawBuffer[1024] = {0};
    char inputString[256] = {0};

    RtAudio adac;
    if(adac.getDeviceCount() < 1){
        puts("No audio devices found :(");
        return 1;
    }

    // Network configuration

    s32 sockStatus = 0;
    do{
        s32 peerPort = 0;
        puts("Uploading? y/n");
        scanf("%s", inputString);
        if(strstr(inputString, "y")){
            puts("Uploading...");
            isUploading = true;
        }
        puts("Enter peer IP");
        scanf("%s", inputString);
        puts("Enter peer port");
        scanf("%i", &peerPort);
        sockStatus = sock.connect(inputString, peerPort, !isUploading);
        puts(sock.status_string(sockStatus));
        puts("--------------------------------------------------------------");
    }
    while(sockStatus != SOCK_OK);

    std::thread net_thread(networkCB);

    // Audio configuration

    const u32 numDevices = adac.getDeviceCount();
    constexpr u32 sample_rate = 44100;
    u32 buffBytes, buffFrames = networkBufferLen;
    RtAudio::StreamParameters inParams, outParams;

    bool validConfig = false;
    while(!validConfig){
        RtAudio::DeviceInfo dInfo;
        for(u32 i = 0; i < numDevices; i++){
            dInfo = adac.getDeviceInfo(i);
            
            if(dInfo.probed){
                printf("device id: %u\n", i);
                puts(dInfo.name.c_str());
                printf("output channels: %u\n", dInfo.outputChannels);
                printf("input channels: %u\n", dInfo.inputChannels);
                if(dInfo.isDefaultInput){
                    puts("*default input device*");
                    inParams.deviceId = i;
                }
                if(dInfo.isDefaultOutput){
                    puts("*default output device*");
                    outParams.deviceId = i;
                }
                puts("--------------------------------------------------------------");
            }
        }
        
        puts("Use default devices?");
        scanf("%15s", inputString);
        if(!strstr(inputString, "y")){
            puts("Choose input device Id");
            scanf("%u", &inParams.deviceId);
            puts("Choose output device Id");
            scanf("%u", &outParams.deviceId);
            inParams.deviceId %= numDevices;
            outParams.deviceId %= numDevices;
        }

        inParams.nChannels = adac.getDeviceInfo(inParams.deviceId).inputChannels;
        outParams.nChannels = adac.getDeviceInfo(outParams.deviceId).outputChannels;

        try{
            adac.openStream(&outParams, &inParams, RTAUDIO_FLOAT32, sample_rate, &buffFrames, &audioCB, (void*)&buffBytes);
            validConfig = true;
        }
        catch(RtAudioError& e){
            puts("Open stream error: ");
            e.printMessage();
            validConfig = false;
        }
    }

    // Audio stream begin

    buffBytes = buffFrames * outParams.nChannels * sizeof(float);

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

    puts("Running... enter q to quit");
    scanf("%15s", inputString);
    while(adac.isStreamOpen() && !strstr(inputString, "q")){
        scanf("%15s", inputString);
    }

    puts("Quitting...");
    netrun = false;

    if(adac.isStreamOpen()){
        adac.closeStream();
    }

    return 0;
}