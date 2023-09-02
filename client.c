#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#define MAX_MSG_SIZE 1024
#define MAX_QUEUE_SIZE 16

void error(char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char const *argv[]) {

    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(50000);

    if (connect(sock_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1)
    {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    char msg[MAX_QUEUE_SIZE][MAX_MSG_SIZE];
    char buffer[MAX_MSG_SIZE];

    int queue_size = 0;
    int first_idx = 0;
    int last_idx = MAX_QUEUE_SIZE-1;

    while (1) {
        printf("Enter message: ");
        last_idx = (++last_idx)&(MAX_QUEUE_SIZE-1);
        fgets(msg[last_idx], MAX_MSG_SIZE, stdin);
        queue_size++;

        if (strcmp(msg[last_idx], "bye\n") == 0) {
            if(send(sock_fd, "Echo_CLOSE", strlen("Echo_CLOSE"), 0) < 0) error("send error");
            if(recv(sock_fd, buffer, MAX_MSG_SIZE, 0) < 0) error("recv error");
            close(sock_fd);
            break;
        }
        else if(strcmp(msg[last_idx], "Q\n")==0)
        {
            memset(msg[last_idx], 0, MAX_MSG_SIZE);
            last_idx = (last_idx+MAX_QUEUE_SIZE-1)&(MAX_QUEUE_SIZE-1);

            if(send(sock_fd, "SEND", MAX_MSG_SIZE, 0) < 0) error("send error");
            int tmp = queue_size;
            while(--tmp)
            {
                if(send(sock_fd, msg[first_idx], MAX_MSG_SIZE, 0) < 0) error("send error");
                memset(msg[first_idx], 0, MAX_MSG_SIZE);
                first_idx = (++first_idx)&(MAX_QUEUE_SIZE-1);
            }
            if(send(sock_fd, "RECV", MAX_MSG_SIZE, 0) < 0) error("send error");
            printf("< Server responses >\n");
            while(--queue_size)
            {
                if(recv(sock_fd, buffer, MAX_MSG_SIZE, 0) < 0) error("recv error");
                printf("Server response: %s", buffer);
                memset(buffer, 0, MAX_MSG_SIZE);
            }
            first_idx = 0;
            last_idx = MAX_QUEUE_SIZE-1;
            queue_size = 0;
            printf("\n------< New message >------\n");
        }
        else printf("Messaged queued.\n");
    }

    return 0;
}
