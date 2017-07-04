#include "socks.h"
#include <cstdio>
#include <cassert>

#ifdef _WIN32 // WINDOWS 
#include <winsock2.h>
#include <Ws2tcpip.h>

struct udp_impl{
    SOCKET sock;

    int Connect(const char* addr, int port, bool server){
        WSADATA wsa;
        struct sockaddr_in peer;
        if(WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
            return SOCK_FAIL_STARTUP;

        sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if(sock == INVALID_SOCKET)
            return SOCK_FAIL_SOCKET;

        if(InetPton(AF_INET, addr, &peer.sin_addr.s_addr) != 1)
            return SOCK_FAIL_ADDR;
        
        peer.sin_family = AF_INET;
        peer.sin_port = htons(port);

        if(server && bind(sock, (struct sockaddr *)&peer, sizeof(peer)) < 0)
            return SOCK_FAIL_BIND;

        if(!server && connect(sock, (struct sockaddr *)&peer, sizeof(peer)) < 0)
            return SOCK_FAIL_CONNECT;

        return 0;
    }

    int Send(const char* buf, int len){
        return send(sock, buf, len, 0);
    }

    int Recv(char* out, int len){
        return recv(sock, out, len, 0);
    }
};

#elif defined(_POSIX_VERSION) // POSIX


#endif // PLATFORM SPECIFIC CODE

static_assert(sizeof(udp_impl) >= sizeof(udp_socket));

const char* sock_status_str[] = {
    "Socket OK",
    "Socket Failed Startup",
    "Socket Failed Socket",
    "Socket Failed Address",
    "Socket Failed Connect",
    "Socket Failed Bind",
    "Smelly Socks :S"
};

const char* udp_socket::status_string(int code){
    return sock_status_str[code];
}
int udp_socket::send(const void* buf, int len){
    udp_impl* pimpl = (udp_impl*)&impl;
    return pimpl->Send((const char*)buf, len);
}
int udp_socket::recv(void* out, int len){
    udp_impl* pimpl = (udp_impl*)&impl;
    return pimpl->Recv((char*)out, len);
}
int udp_socket::connect(const char* addr, int port, bool isServer){
    udp_impl* pimpl = (udp_impl*)&impl;
    return pimpl->Connect(addr, port, isServer);
}