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

/*
//For reference purpose only
struct sockaddr_in {
        short   sin_family;
        u_short sin_port;
        struct  in_addr sin_addr;
        char    sin_zero[8];
};
*/
int main(int argc, char const *argv[]) 
{ 
    int server_fd, new_socket, valread, sd, max_sd, activity, i;
    int max_clients = 3;
    int client_sockets[3];
    struct sockaddr_in address; 
    struct sockaddr_in client_address; 
    int port = 8080;
    int opt = 1; 
    int addrlen = sizeof(address); 
    char buffer[1025] = {0};
    char *hello = "Hello from server";

    for (i = 0; i < max_clients; i++)
    {
        client_sockets[i] = 0;
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

        for (i = 0; i < max_clients; i++) {
            sd = client_sockets[i];

            if (sd > 0)
                FD_SET(sd, &master_fd);

            if (sd > max_sd)
                max_sd = sd;
        }

        activity = select(max_sd + 1, &master_fd, NULL, NULL, NULL);

        if (FD_ISSET(server_fd, &master_fd)) {

            if ((new_socket = accept(server_fd,
                                     (struct sockaddr *) &address, (socklen_t * ) & addrlen)) < 0) {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            char * message = "You are successfully connected";


            if (send(new_socket, message, strlen(message), 0) != strlen(message)) {
                perror("send");
            }

            for (i = 0; i < max_clients; i++) {
                //if position is empty
                if (client_sockets[i] == 0) {
                    client_sockets[i] = new_socket;
                    break;
                }
            }

        }
        for (i = 0; i < max_clients; i++)
        {
            sd = client_sockets[i];
            if (FD_ISSET(sd , &master_fd))
            {
                //Check if it was for closing , and also read the
                //incoming message
                if ((valread = read( sd , buffer, 1024)) == 0)
                {
                    //Somebody disconnected , get his details and print
                    /*
                    getpeername(sd , (struct sockaddr*)&address , \
                        (socklen_t*)&addrlen);
                    printf("Host disconnected , ip %s , port %d \n" ,
                           inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

                    */
                    //Close the socket and mark as 0 in list for reuse
                    printf("Host disconnected");
                    close(sd);
                    client_sockets[i] = 0;
                }
                else
                {
                    //set the string terminating NULL byte on the end
                    //of the data read
                    buffer[valread] = '\0';
                    printf("Message : %s\n", buffer);
                }
            }
        }
    }
} 
