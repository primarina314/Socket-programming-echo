#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_BUFFER_SIZE 1024
#define MAX_QUEUE_SIZE 16

void error(char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    char buffer[MAX_QUEUE_SIZE][MAX_BUFFER_SIZE];
    
    int read_size;
    int first_idx = 0;
    int last_idx = MAX_QUEUE_SIZE-1;

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        printf("Error creating socket\n");
        exit(1);
    }
    printf("Socket created\n");

    // Prepare the sockaddr_in structure
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(50000);

    // Bind
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        printf("Error binding\n");
        exit(1);
    }
    printf("Bind done\n");

    // Listen
    listen(server_socket, 3);

    // Accept and incoming connection
    printf("Waiting for incoming connections...\n");
    socklen_t client_address_size = sizeof(client_address);
    client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_size);
    if (client_socket < 0) {
        printf("Error accepting connection\n");
        exit(1);
    }
    printf("Connection accepted\n");

    while (1) {
        last_idx = (++last_idx)&(MAX_QUEUE_SIZE-1);
        read_size = recv(client_socket, buffer[last_idx], MAX_BUFFER_SIZE, 0);
        if(read_size < 0) error("recv error");
        buffer[last_idx][read_size] = '\0';
        
        if(strcmp(buffer[last_idx], "Echo_CLOSE")==0)
        {
            printf("---Echo_CLOSE---\n");
            if(send(client_socket, "Echo_CLOSE", MAX_BUFFER_SIZE, 0) < 0) error("send error");
            break;
        }
        else if(strcmp(buffer[last_idx], "SEND")==0)
        {
            printf("------SEND------\n");
            while(1)
            {
                last_idx = (++last_idx)&(MAX_QUEUE_SIZE-1);
                read_size = recv(client_socket, buffer[last_idx], MAX_BUFFER_SIZE, 0);
                if(read_size < 0) error("recv error");
                buffer[last_idx][read_size] = '\0';
                if(strcmp(buffer[last_idx], "RECV")==0) break;
                printf("Client said: %s", buffer[last_idx]);
            }
            while(strcmp(buffer[first_idx], "RECV")!=0)
            {
                if(strcmp(buffer[first_idx], "SEND")==0)
                {
                    first_idx = (++first_idx)&(MAX_QUEUE_SIZE-1);
                    continue;
                }
                if(send(client_socket, buffer[first_idx], MAX_BUFFER_SIZE, 0) < 0) error("send error");
                first_idx = (++first_idx)&(MAX_QUEUE_SIZE-1);
            }
            printf("------RECV------\n\n");
        }
        first_idx = 0;
        last_idx = MAX_QUEUE_SIZE-1;
    }

    if (read_size == 0) printf("Client disconnected\n");
    else if (read_size == -1)
    {
        printf("Error receiving message from client\n");
        exit(1);
    }

    close(client_socket);
    close(server_socket);

    return 0;
}
