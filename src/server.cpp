
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
#include "parser.h"
#include "user.h"
#include "networking.h"


#define CONFIG_FILE "grass.conf"
// new include here (cpp related)
#include <arpa/inet.h>
#include <set>
#include <vector>
#include <map>

using namespace std;

void runServer(uint16_t port, parser parser);

size_t split(vector<string> &res, const string &line, char delim){
    size_t pos = line.find(delim);
    size_t initialPos = 0;
    res.clear();
    while( pos != string::npos){
        res.push_back(line.substr(initialPos, pos-initialPos));
        initialPos = pos + 1;
        while ((pos = line.find(delim, initialPos)) == initialPos){
            initialPos += 1;
        }
    }
    res.push_back(line.substr(initialPos, min(pos, line.size() - initialPos + 1)));
    return res.size();
}
// FIXME file << ... instead of printf
int main()
{
    // Start parser code
    parser parser;
    parser.initialize();
    // End parser code

    //TODO get necessary info from conf file

    // start the server
    runServer(8080, parser);
}

void runServer(uint16_t port, parser parser){
    int server_fd, new_socket, sd, max_sd, activity;
    ssize_t  valread;
    struct sockaddr_in address;
    int opt = 1;
    int transferPort = 5000;
    int addrlen = sizeof(address);
    char buffer[1025] = {0};
    set<user> connected_users;

    string baseDir;
    map<string, string> knownUsers;

    cout << "parsing config file \n";
    // Parsing config file
    ifstream configFile(CONFIG_FILE);
    string line; //FIXME we could add vulnerabilities in the config file
    vector<string> splitLine;
    if(configFile.is_open()){
        while(getline(configFile, line)){
            split(splitLine, line, ' ');
            if(splitLine[0] == "base"){
                baseDir = splitLine[1];
            }
            if(splitLine[0] == "port"){
                port = stoi(splitLine[1]);
            }
            if(splitLine[0] == "user"){
                knownUsers.insert(pair<string, string>(splitLine[1], splitLine[2]));
            }

        }
    }

    cout << "Running on port: " << port << " , " << "base directory: " << baseDir << "\n";
    cout << "Known users : \n";
    for (const auto &knownUser : knownUsers) {
        std::cout << knownUser.first << " -> " << knownUser.second << "\n";
    }


    // Creating socket file descriptor
    // AF_INET: IPv4, SOCK_STREAM : TCP, 0 : IP
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket creation error");
        exit(1);
    }

    if( setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
                   sizeof(opt)) < 0 )
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = INADDR_ANY;

    // Binds
    if (bind(server_fd, (struct sockaddr *)&address,
             sizeof(address))<0)
    {
        perror("binding error");
        exit(1);
    }

    // 5 is the maximum is the size of the backlog queue (5 is the limit on most systems)
    if (listen(server_fd, 5) < 0)
    {
        perror("listen error");
        exit(1);
    }

    fd_set master_fd;
    while(true) {
        FD_ZERO(&master_fd);
        FD_SET(server_fd, &master_fd);
        max_sd = server_fd;
        for (auto connected_user : connected_users) {
            sd = connected_user.getSocket();

            if (sd > 0)
                FD_SET(sd, &master_fd);

            if (sd > max_sd)
                max_sd = sd;
        }

        // waits for one of the socket to receive some activity
        activity = select(max_sd + 1, &master_fd, nullptr, nullptr, nullptr);

        // A new TCP connection has been opened
        if (FD_ISSET(server_fd, &master_fd)) {

            if ((new_socket = accept(server_fd,
                                     (struct sockaddr *) &address, (socklen_t * ) & addrlen)) < 0) {
                perror("accept");
                exit(1);
            }

            string message = "You are successfully connected young padawan";
            printf("New connection , socket fd is %d , ip is : %s , port : %d\n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

            if (send(new_socket, message.c_str(), strlen(message.c_str()), 0) != strlen(message.c_str())) {
                perror("send");
                exit(1);
            }

            user newUsr = user(new_socket);
            connected_users.insert(newUsr);
        }
        for (auto it = connected_users.begin(); it != connected_users.end(); )
        {   sd = (*it).getSocket();
            if (FD_ISSET(sd , &master_fd))
            {
                //Check if it was for closing, and also read the
                //incoming message
                if ((valread = read( sd , buffer, 1024)) == 0)
                {
                    //Host disconnected
                    getpeername(sd , (struct sockaddr*)&address , \
                        (socklen_t*)&addrlen);
                    printf("Host disconnected , ip %s , port %d \n" ,
                           inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

                    //Close the socket and mark as 0 in list for reuse
                    close(sd);
                    connected_users.erase(it);
                }
                else
                {
                    //set the string terminating NULL byte on the end
                    //of the data read
                    buffer[valread] = '\0';
                    // buffer contains the received command
                    printf("%s\n", buffer);
                    // response to be sent
                    string message;


                    parser.parseCommand(buffer);
                    parser.executeCommand(*it);
                    parser.resetCommand();
                    message = parser.getOutput();

                    /*
                       End Parser code
                    */

                    if (send(sd, message.c_str(), strlen(message.c_str()), 0) != strlen(message.c_str())) {
                        perror("send");
                    }
                }
            }
            ++it;
        }
    }
}