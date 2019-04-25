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

// new include here (cpp related)
#include <arpa/inet.h>
#include <set>
#include <vector>
#include <regex>
#include <unistd.h>
#define DEFAULT_MODE_ARGC 3
#define AUTO_MODE_ARGC 5
#define IP_SIZE 32

using namespace std;
int runClient(char* serverIp, uint16_t serverPort, istream& infile, ostream& outfile, bool automated_mode);

int main( int argc, const char* argv[] )
{    bool automated_mode = (argc == AUTO_MODE_ARGC);

    if(argc != DEFAULT_MODE_ARGC and !automated_mode){
        cerr << "Expected command: ./client server-ip server-port [infile outfile]\n";
        return -1;
    };

    // parsing server IP
    char serverIp[1024];
    strncpy(serverIp, argv[1], IP_SIZE);

    // parsing server port
    int tmp = stoi(argv[2]);
    uint16_t serverPort(0);
    if (tmp <= static_cast<int>(UINT16_MAX) && tmp >=0) {
        serverPort = static_cast<uint16_t>(tmp);
    }
    else {
        cout << "Error: server port cannot be cast to uint16\n";
        return 1;
    }

    // parsing input/output files
    istream& infile = automated_mode ? *(new ifstream(argv[3])) : cin;
    ostream& outfile = automated_mode ? *(new ofstream(argv[4])) : cout;

    runClient(serverIp, serverPort, infile, outfile,  automated_mode);

    return 0;
}

int runClient(char* serverIp, uint16_t serverPort, istream& infile, ostream& outfile, bool automated_mode){

    //Local variables setup
    int mainSocket = 0;
    struct sockaddr_in serverAddress;

    char buffer[1024] = {0};
    char copiedBuffer[1024] = {0};
    char cmdCopy[1024];
    char *token;
    string cmd;

    pthread_t thread;

    char fileName[1024];
    long fileSize = 0;
    int port = 0;

    int attempts = 0;
    bool connected = false;
    while(attempts < 10 and !connected) {
        // Network setup
        if ((mainSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            printf("Socket creation error");
            return 1;
        }

        memset(&serverAddress, 0, sizeof(serverAddress));

        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(serverPort);

        if (inet_pton(AF_INET, serverIp, &serverAddress.sin_addr) <= 0) {
            printf("Address error");
            return 1;
        }

        if (connect(mainSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) == 0) {
            connected = true;
        }else {
            sleep(1);
        }
    }

    if(!connected){
        return 1;
    }

    while(true){
        // FIXME added again temporarily for readability
        if(!automated_mode) {
            cout << ">> ";
        }

        getline(infile, cmd);

        // Check if end of file reached, or exit command sent
        if(infile.eof() || cmd == "exit"){
            outfile << "\n[EOF reached]\n"; // FIXME probably need to delete this as not in specifications
            // closing connection
            close(mainSocket);
            break;
        }

        // This line avoids sending an empty line and waiting indefinitely
        if(cmd.empty()) {
            continue;
        }

        // Extract filename from get/put command if applicable
        strncpy(cmdCopy, cmd.c_str(), 1024);
        token = strtok(cmdCopy, " ");
        if(strcmp(token, "get") == 0){
            memset(fileName, 0, 1024);
            strncpy(fileName, strtok(NULL, " "), 1024);
        }else if(strcmp(token, "put") == 0){
            memset(fileName, 0, 1024);
            strncpy(fileName, strtok(NULL, " "), 1024);
            //FIXME SEGFAULT WHEN NULL
            fileSize = stol(strtok(NULL, " "));
        }

        // sends command to the server
        send(mainSocket , cmd.c_str(), strlen(cmd.c_str()) , 0);

        // wait for server to respond to the command sent
        read(mainSocket , buffer, 1024);
        strncpy(copiedBuffer, buffer, 1024);

        string bufString = string(buffer);

        // trim string, since we send empty spaces when command returns nothing. FIXME
        bufString.erase(0, bufString.find_first_not_of(' '));
        outfile << bufString;
        memset(buffer, 0, 1024);
        token = strtok(copiedBuffer, " ");

        /*
         * When the message received by the server corresponds to a get command response, a thread
         * is created to receive the requested file in parallel
         */
        if(token and strcmp(token, "get") == 0 and strcmp(strtok(NULL, " "), "port:") == 0){
            // Extract port and fileSize from received command string
            port = atoi(strtok(NULL, " "));
            strtok(NULL, " ");
            fileSize = stol(strtok(NULL, " "));

            // Prepare struct for argument to function in parallel thread
            struct thread_args *args = (struct thread_args *)malloc(sizeof(struct thread_args));
            strncpy(args->fileName, fileName, 1024);
            args->port = port;
            args->fileSize = fileSize;
            strncpy(args->ip, serverIp, 1024);

            // Kill stale thread
            pthread_cancel(thread);

            // Create new thread
            int rc = pthread_create(&thread, NULL, openFileClient, (void*) args);
            if(rc != 0){
                cerr << "Error" << endl;
            }
        }else if(token and strcmp(token, "put") == 0 and strcmp(strtok(NULL, " "), "port:") == 0){
            // Extract port and fileSize from received command string
            port = atoi(strtok(NULL, " "));
            // Prepare struct for argument to function in parallel thread
            struct thread_args *args = (struct thread_args *)malloc(sizeof(struct thread_args));
            strncpy(args->fileName, fileName, 1024);
            args->port = port;
            args->fileSize = fileSize;

            // Kill stale thread
            pthread_cancel(thread);
            // Create new thread
            int rc = pthread_create(&thread, NULL, openFileServer, (void*) args);
            if(rc != 0){
                cerr << "Error" << endl;
            }
        }
    }


    if(automated_mode){
        // FIXME does delete() close fstreams ?
        delete(&infile);
        delete(&outfile);
    }
    return 0;
}