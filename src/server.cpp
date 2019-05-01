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
#include <stdio.h>
//#include <filesystem>
#include "grass.hpp"

#define CONFIG_FILE "grass.conf"
#define BUF_SIZE 900
#define MAX_READ_LEN 650
using namespace std;


// Global variables
set<User> connected_users;
string baseDirectory;


int runServer(uint16_t port, Parser &parser);
int executeCommand(ssize_t &valread, Parser &parser, std::_Rb_tree_const_iterator<User> &it, struct sockaddr_in address, size_t addressLength);
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
    int res = runServer(port, parser);
    return res;
}

int runServer(uint16_t port, Parser &parser) {
    int mainSocket, newSocket, sd, max_sd;
    ssize_t valread;
    struct sockaddr_in address{};
    int opt = 1;
    size_t addressLength = sizeof(address);

    // Creating server socket
    if ((mainSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        cleanBaseDir();
        return 1;
    }

    if (setsockopt(mainSocket, SOL_SOCKET, SO_REUSEADDR, (char *) &opt,
                   sizeof(opt)) < 0) {
        cleanBaseDir();
        return 1;
    }

    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = INADDR_ANY;

    // Binding server socket
    if (bind(mainSocket, (struct sockaddr *) &address,
             sizeof(address)) < 0) {
        cleanBaseDir();
        return 1;
    }

    // Listen to incoming connections
    if (listen(mainSocket, 5) < 0) {
        cleanBaseDir();
        return 1;
    }

    // Main loop which detects any change in the file descriptors corresponding to received messages
    fd_set master_fd;
    while (true) {
        FD_ZERO(&master_fd);
        FD_SET(mainSocket, &master_fd);
        max_sd = mainSocket;
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
        if (FD_ISSET(mainSocket, &master_fd)) {

            if ((newSocket = accept(mainSocket,
                                    (struct sockaddr *) &address, (socklen_t *) &addressLength)) < 0) {
                cleanBaseDir();
                return 1;
            }

            // Create new user
            User newUsr = User(newSocket, string(inet_ntoa(address.sin_addr)), "");
            connected_users.insert(newUsr);
        }

        // Loop over connected users and detect incoming messages
        for (auto it = connected_users.begin(); it != connected_users.end();) {
            sd = (*it).getSocket();

            if (FD_ISSET(sd, &master_fd)) {
                if (executeCommand(valread, parser, it, address, addressLength) != 0) {
                    return 1;
                }
            } else {
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


int executeCommand(ssize_t &valread, Parser &parser, std::_Rb_tree_const_iterator<User> &it, struct sockaddr_in address, size_t addressLength) {
    char buffer[BUF_SIZE+1];
    int sd =  (*it).getSocket();
    //Read the incoming message and check whether it is for closing a connection
    if ((valread = read(sd, buffer, BUF_SIZE)) == 0)
    {
        //Host disconnected
        getpeername(sd , (struct sockaddr*) &address , (socklen_t*) &addressLength);

        //Close the socket and delete user from list
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
        parser.parseCommand(buffer);

        parser.executeCommand(const_cast<User &>(*it));

        //strcpy(message, string(parser.getOutput()).empty()? " ": parser.getOutput());
        string message = parser.getOutput().empty()? " ": parser.getOutput();
        bool shouldPrint = parser.getShouldPrint();
        // Send response to client
        if ((int)send(sd, message.c_str(), message.size(), 0) != (int) message.size()) {
            return 1;
        }
        // trim output
        if(shouldPrint)
            cout << message.erase(0, string(message).find_first_not_of(' '));

        parser.resetCommand();
        ++it;
    }
    return 0;
}
