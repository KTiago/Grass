//
// Created by tiago on 25.03.19.
//


#include<iostream>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/stat.h>
#include "Parser.h"
#include "User.h"
#include "networking.h"
#include <arpa/inet.h>
#include <set>
#include <vector>
#include <unistd.h>

#define SOCKET int
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

/*
 * Code inspired from
 * https://stackoverflow.com/questions/25634483/send-binary-file-over-tcp-ip-connection
 * https://www.geeksforgeeks.org/socket-programming-in-cc-handling-multiple-clients-on-server-without-multi-threading/
 */

using namespace std;

// Creates a TCP server to serve a file. Arguments are passed in a struct.
void* openFileServer(void* ptr){
    struct thread_args *args = (struct thread_args *)ptr;
    FILE *f = fopen(args->fileName, "r");
    int port = args->port;

    struct sockaddr_in address;
    int newSocket, mainSocket;
    int opt = 1;
    int addressLength = sizeof(address);
    if ((mainSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0){return (void*)1;}

    if(setsockopt(mainSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
                   sizeof(opt)) < 0 ){return (void*)1;}

    // Binds
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = INADDR_ANY;

    if (bind(mainSocket, (struct sockaddr *)&address,
             sizeof(address))<0){return (void*)1;}


    if (listen(mainSocket, 1) < 0){return (void*)1;}


    if ((newSocket = accept(mainSocket, (struct sockaddr *)&address,
                             (socklen_t*)&addressLength))<0){return (void*)1;}


    if(!sendFile(newSocket, f)){return (void*)1;}

    fclose(f);
    close(newSocket);
    close(mainSocket);
    return (void*)0;
}

// Creates a TCP client to retrieve a file from a TCP server. Arguments are passed in a struct.
void* openFileClient(void *ptr){
    struct thread_args *args = (struct thread_args *)ptr;
    FILE *f = fopen(args->fileName, "w");
    char* serverIp = args->ip;
    int port = args->port;
    long fileSize = args->fileSize;

    int  main_socket;

    sockaddr_in address;
    int attempts = 0;
    bool connected = false;

    // Attempts multiple times to connect to server. Waits 1s between each attempt.
    while(attempts < 10 and !connected) {
        if ((main_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) { return (void *) 1; }

        memset(&address, 0, sizeof(address));
        address.sin_family = AF_INET;
        address.sin_port = htons(port);

        if (inet_pton(AF_INET, serverIp, &address.sin_addr) <= 0) {
            printf("Address error\n");
            return (void *) 1;
        }


        if (connect(main_socket, (struct sockaddr *) &address, sizeof(address)) == 0) {
            connected = true;
        }
        else{
            sleep(1);
        }
    }
    if(!connected){
        return (void*)1;
    }

    // Reads whole file from server
    readFile(main_socket, f, fileSize);

    fclose(f);
    close(main_socket);
    return (void*)0;
}

bool sendData(SOCKET sock, void *buffer, int bufferLength)
{
    unsigned char *pbuf = (unsigned char *) buffer;

    while (bufferLength > 0)
    {
        int num = send(sock, pbuf, bufferLength, 0);
        if (num == -1)
        {
            return false;
        }

        pbuf += num;
        bufferLength -= num;
    }
    return true;
}

bool sendFile(SOCKET sock, FILE *file)
{
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);
    if (fileSize == EOF)
        return false;
    if (fileSize > 0)
    {
        char buffer[1024];
        do
        {
            size_t num = MIN(fileSize, sizeof(buffer));
            num = fread(buffer, 1, num, file);
            if (num < 1)
                return false;
            if (!sendData(sock, buffer, num))
                return false;
            fileSize -= num;
        }
        while (fileSize > 0);
    }
    return true;
}

bool readData(SOCKET sock, void *buffer, int bufferLength)
{
    unsigned char *pbuf = (unsigned char *) buffer;

    while (bufferLength > 0)
    {
        int num = recv(sock, pbuf, bufferLength, 0);
        if (num == -1 or num == 0)
            return false;

        pbuf += num;
        bufferLength -= num;
    }

    return true;
}

bool readFile(SOCKET sock, FILE *file, long fileSize)
{
    if (fileSize > 0)
    {
        char buffer[1024];
        do
        {
            int num = MIN(fileSize, sizeof(buffer));
            if (!readData(sock, buffer, num))
                return false;
            int offset = 0;
            do
            {
                size_t written = fwrite(&buffer[offset], 1, num-offset, file);
                if (written < 1)
                    return false;
                offset += written;
            }
            while (offset < num);
            fileSize -= num;
        }
        while (fileSize > 0);
    }
    return true;
}