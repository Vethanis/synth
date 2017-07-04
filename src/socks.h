#pragma once

#include "ints.h"

enum sock_status {
    SOCK_OK,
    SOCK_FAIL_STARTUP,
    SOCK_FAIL_SOCKET,
    SOCK_FAIL_ADDR,
    SOCK_FAIL_CONNECT,
    SOCK_FAIL_BIND,
    SOCK_SMELLY
};

struct udp_socket{
    char impl[8];
    //returns number of bytes sent
    int send(const void* buf, int len);
    // returns number of bytes received
    int recv(void* out, int len);
    // addr: ipv4 address string
    // port: socket port
    // returns sock_status
    int connect(const char* addr, int port, bool isReceiver);
    const char* status_string(int code);
};