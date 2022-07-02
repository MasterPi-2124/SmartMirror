#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <limits.h>

#include "client.h"

void send_cmd(int socketFd, char* cmd, int param)
{
    int n;
    char* buff = (char*)calloc(BUFFER_SIZE, 1);
    sprintf(buff, "%s %d", cmd, param); 
    printf("send command: %d\n", buff);
    n = write(socketFd, buff, BUFFER_SIZE);
    printf("write return %d: %s\n", n, strerror(errno));
    free(buff);
}

int recv_cmd(char* buff)
{
    int n;
    printf("buff: %s\n", buff);
    char* cm1 = (char*)calloc(BUFFER_SIZE, 1);
    char* cm2 = (char*)calloc(BUFFER_SIZE, 1);
    int motion_type = INT_MIN;
    int status_code = INT_MIN;
    n = sscanf(buff, "%s %d %s %d", cm1, &status_code, cm2, &motion_type);
    printf("%s\n%d\n%s\n%d\n", cm1, status_code, cm2, motion_type);

    if (status_code != RESP_CODE_OK)
    {
        if(status_code == INT_MIN) n = CL_FAILED;
        n = status_code;
    } else {
        if(strcmp(cm2, RESP_MOTION_LABEL) < 0)
        {
            n = CL_FAILED;
        }
        n = motion_type;
    }
    
    free(cm1);
    free(cm2);
    free(buff);
    return n; 
}

int client_init(int *pSocket)
{
    int n;
    int clientFd;
    clientFd = socket(AF_INET, SOCK_STREAM, 0);
    printf("socket returned %d: %s\n", clientFd, strerror(errno));
    if(clientFd == -1) return CL_FAILED;

    struct sockaddr_in saddr, caddr;
    saddr.sin_family        = AF_INET;
    saddr.sin_port          = htons(SERVER_PORT);
    saddr.sin_addr.s_addr   = inet_addr(SERVER_IP);

    n = connect(clientFd, (struct sockaddr*)&saddr, sizeof(saddr));
    printf("connect returned %d: %s\n", n, strerror(errno));
    if(n == -1) return CL_FAILED;
    *pSocket = clientFd;
    return CL_SUCCESS;
}


int client_deinit(int *pSock)
{
    int n;
    n = close(*pSock);
    printf("close returned %d: %s\n", n, strerror(errno));
    if(n == -1) return CL_FAILED;
    return CL_SUCCESS;
}