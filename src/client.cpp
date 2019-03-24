#include <iostream>
#include <fstream>
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

// new include here (cpp related)
#include <arpa/inet.h>


#define DEFAULT_MODE_ARGC 3
#define AUTO_MODE_ARGC 5
#define IP_SIZE 32

using namespace std;

// FIXME file << ... instead of printf
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

    //Network setup
    int sock = 0;
    ssize_t valread;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Socket creation error");
        return 1;
    }

    // FIXME '0' or 0 ?
    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(serverPort);

    if(inet_pton(AF_INET, serverIp, &serv_addr.sin_addr)<=0)
    {
        printf("Address error");
        return 1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("Connection error\n");
        return 1;
    }

    valread = read(sock , buffer, 1024);
    printf("%s\n", buffer);
    memset(buffer, 0, 1024);
    string cmd;

    while(true){
        if(!automated_mode) {
            cout << ">> ";
        }
        getline(infile, cmd);
        if(infile.eof()){
            outfile << "\n[EOF reached]\n";
            // closing connection
            close(sock);
            break;
        }
        // sends command to the server
        send(sock , cmd.c_str(), strlen(cmd.c_str()) , 0);

        // server response to the command sent
        valread = read(sock , buffer, 1024);
        printf("%s\n",buffer);
        memset(buffer, 0, 1024);
        // outfile << cmd << "\n";
    }


    if(automated_mode){
        // FIXME does delete() close fstreams ?
        delete(&infile);
        delete(&outfile);
    }

    return 0;
}

