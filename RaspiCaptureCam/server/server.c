#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "server.h"

int socket_init(int *pSocket)
{
    int n;
    int connSockFd, sockFd;
    sockFd = socket(AF_INET, SOCK_STREAM, 0);
    printf("[socket]\t\treturned %d: %s\n", sockFd, strerror(errno));
    if(sockFd == -1)
    {
        return SV_FAILED;
    }

    struct sockaddr_in saddr, caddr;
    saddr.sin_family        = AF_INET;
    saddr.sin_port          = htons(SERVER_PORT);
    saddr.sin_addr.s_addr   = INADDR_ANY;

    //bind
    n = bind(sockFd, (struct sockaddr*)&saddr, sizeof(struct sockaddr));
    printf("[bind]\t\treturned %d: %s\n", n, strerror(errno));
    if (n == -1) return SV_FAILED;

    //listen
    n = listen(sockFd, LISTEN_BACKLOG);
    printf("[listen]\t\treturned %d: %s\n", n, strerror(errno));
    if(n == -1) return SV_FAILED;

    //accept
    socklen_t clen = sizeof(caddr);
    connSockFd = accept(sockFd, (struct sockaddr*)&caddr, &clen);
    printf("[accept]\t\treturned %d: %s\n", connSockFd, strerror(errno));
    if(connSockFd == -1) return SV_FAILED;

    *pSocket = connSockFd;

   return SV_SUCCESS; 
}

int socket_deinit(int *pSocket)
{
    int n;
    n = close(*pSocket);
    printf("[close]\t\treturned %d: %s\n", n, strerror(errno));
    if (n != 0) return SV_FAILED;
    return SV_SUCCESS;
}

int main(int argc, char* argv[])
{
    int n;
    int socket;
    char* buffer;
    buffer = (char*)calloc(BUFFER_SIZE, 1);

    socket_init(&socket);

    // strcpy(buffer, "OK 200 MOTION 7"); 
    // write(socket, buffer, BUFFER_SIZE);

    // communication
    // step 1: recv request from client
    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);
        n = read(socket, buffer, strlen(REQ_CMD));
        printf("read returned %d , [data: %s]: %s\n", n, buffer, strerror(errno));
        n = strcmp(buffer, REQ_CMD);
        printf("strcmp returned %d: %s\n", n, strerror(errno));
        if (n >= 0) break;
    }

    // step 2: send ack to client
    memset(buffer, 0, BUFFER_SIZE);
    sprintf(buffer, "%s %d", RESP_CMD, RESP_CODE_OK);
    write(socket, buffer, strlen(buffer));
    printf("write returned %d , [data: %s]: %s\n", n, buffer, strerror(errno));
    // step 3: recv img from client

    // step 4: send ack to client
    // step 5: send response to client

    // end communication

    free(buffer);

    socket_deinit(&socket);
    
    return 0;
}