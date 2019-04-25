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
#include "commands.h"

// new include here (cpp related)
#include <arpa/inet.h>
#include <set>
#include <vector>
#include <map>
//#include <filesystem>
#include "grass.hpp"

#define CONFIG_FILE "grass.conf"

using namespace std;


// Global variables
set<User> connected_users;
string baseDirectory;


void runServer(uint16_t port, Parser parser);
void cleanBaseDir();

int main()
{
    uint16_t port;
    map<string, string> allowedUsers;

    // Parsing config file
    ifstream configFile(CONFIG_FILE);
    string line; //FIXME we could add vulnerabilities in the config file
    vector<string> splitLine;
    if(configFile.is_open()){
        while(getline(configFile, line)){
            split(splitLine, line, " ");
            if(splitLine[0] == "base"){
                struct stat info{};
                stat( "baseDir", &info );
                if(info.st_mode & S_IFDIR){
                    system("rm -r baseDir");
                }
                system("mkdir baseDir");
                chdir("baseDir");
                baseDirectory = splitLine[1];
            }
            if(splitLine[0] == "port"){
                port = static_cast<uint16_t>(stoi(splitLine[1]));
            }
            if(splitLine[0] == "user"){
                allowedUsers.insert(pair<string, string>(splitLine[1], splitLine[2]));
            }
        }
    }

    // FIXME works even without config file?
    configFile.close();

    // Create parser object
    Parser parser(allowedUsers);

    // Start the server
    runServer(port, parser);
}

void runServer(uint16_t port, Parser parser){
    int server_fd, new_socket, sd, max_sd;
    ssize_t  valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1025];


    // Creating socket file descriptor
    // AF_INET: IPv4, SOCK_STREAM : TCP, 0 : IP
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket creation error");
        cleanBaseDir();
        exit(1);
    }

    if( setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
                   sizeof(opt)) < 0 )
    {
        perror("setsockopt");
        cleanBaseDir();
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
        cleanBaseDir();
        exit(1);
    }

    // 5 is the maximum is the size of the backlog queue (5 is the limit on most systems)
    if (listen(server_fd, 5) < 0)
    {
        perror("listen error");
        cleanBaseDir();
        exit(1);
    }

    fd_set master_fd;
    while(true) {
        FD_ZERO(&master_fd);
        FD_SET(server_fd, &master_fd);
        max_sd = server_fd;
        for (const auto &connected_user : connected_users) {
            sd = connected_user.getSocket();

            if (sd > 0)
                FD_SET(sd, &master_fd);

            if (sd > max_sd)
                max_sd = sd;
        }

        // waits for one of the socket to receive some activity
        select(max_sd + 1, &master_fd, nullptr, nullptr, nullptr);

        // A new TCP connection has been opened
        if (FD_ISSET(server_fd, &master_fd)) {

            if ((new_socket = accept(server_fd,
                                     (struct sockaddr *) &address, (socklen_t * ) & addrlen)) < 0) {
                perror("accept");
                cleanBaseDir();
                exit(1);
            }

            User newUsr = User(new_socket, string(inet_ntoa(address.sin_addr)),"");
            connected_users.insert(newUsr);

        }
        for (auto it = connected_users.begin(); it != connected_users.end(); )
        {
            sd = (*it).getSocket();
            if (FD_ISSET(sd , &master_fd))
            {
                //Check if it was for closing, and also read the
                //incoming message
                if ((valread = read(sd, buffer, 1024)) == 0)
                {
                    //Host disconnected
                    getpeername(sd , (struct sockaddr*) &address , (socklen_t*) &addrlen);

                    /*
                    printf("Host disconnected , ip %s , port %d \n" ,
                           inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
                    */

                    //Close the socket and mark as 0 in list for reuse
                    close(sd);
                    it = connected_users.erase(it);
                }
                else
                {
                    //set the string terminating NULL byte on the end
                    //of the data read
                    buffer[valread] = '\0';

                    // buffer contains the received command trimmed to 1024 characters
                    // response to be sent

                    /*
                        Yann/Delphine : insert code here to handle the command received and then
                        send the response to the User
                    */

                    parser.parseCommand(buffer);
                    parser.executeCommand(const_cast<User &>(*it));
                    string message = parser.getOutput().empty()? " ": parser.getOutput();
                    parser.resetCommand();

                    if ((int)send(sd, message.c_str(), strlen(message.c_str()), 0) != (int)strlen(message.c_str())) {
                        perror("send");
                    }

                    // trim output
                    message.erase(0, message.find_first_not_of(' '));
                    cout << message;

                    ++it;
                }
            }else{
                ++it;
            }
        }
    }
}


void cleanBaseDir(){
    if(!baseDirectory.empty()){
        system("cd .. ; rm -r baseDir");
    }
}