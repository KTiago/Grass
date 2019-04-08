//
// Created by tiago on 25.03.19.
//

#ifndef GRASS_NETWORKING_H
#define GRASS_NETWORKING_H


//
// Created by tiago on 25.03.19.
//

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <cstdio>
#include <algorithm>

#define SOCKET int

/*
 * Code inspired from
 * https://stackoverflow.com/questions/25634483/send-binary-file-over-tcp-ip-connection
 * https://www.geeksforgeeks.org/socket-programming-in-cc-handling-multiple-clients-on-server-without-multi-threading/
 */

using namespace std;

bool senddata(SOCKET sock, void *buf, int buflen);

bool sendlong(SOCKET sock, long value);

bool sendfile(SOCKET sock, FILE *f);

bool readdata(SOCKET sock, void *buf, int buflen);

bool readlong(SOCKET sock, long *value);

bool readfile(SOCKET sock, FILE *f);

#endif //GRASS_NETWORKING_H