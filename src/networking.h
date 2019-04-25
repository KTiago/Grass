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
struct thread_args
{
    int port;
    //FIXME should serverIp be const char * ?
    char ip[1024];
    char fileName[1024];
    long fileSize;
};

using namespace std;
void* openFileServer(void* ptr);
void* openFileClient(void *ptr);
bool sendData(SOCKET sock, void *buf, int buflen);
bool sendFile(SOCKET sock, FILE *f);
bool readData(SOCKET sock, void *buf, int buflen);
bool readFile(SOCKET sock, FILE *f, long filesize);

#endif //GRASS_NETWORKING_H