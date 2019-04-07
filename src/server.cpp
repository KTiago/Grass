
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

// new include here (cpp related)
#include <arpa/inet.h>
#include <set>
#include <vector>

using namespace std;

// FIXME file << ... instead of printf
int main()
{
    // Start parser code
    parser parser;
    parser.initialize();
    // End parser code

    int server_fd, new_socket, sd, max_sd, activity;
    ssize_t  valread;
    struct sockaddr_in address;
    uint16_t port = 8080;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1025] = {0};
    set<user> connected_users;

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
                    //Host disconnectef disconnecte
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
                    printf("Message : %s\n", buffer);
                    // response to be sent
                    string message = "Command received";


                    /*
                        Yann/Delphine : insert code here to handle the command received and then
                        send the reponse to the user
                    */
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
