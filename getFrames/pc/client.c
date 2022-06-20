#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>

#define SERVER_ADDR ("192.168.1.22")
#define SERVER_PORT (9000)
#define FAILED_ERROR (1)
#define LISTEN_BACKLOG (5)
#define MTU_SIZE (1500)

int main(int argc, char* argv[])
{
    int sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1)
    {
        perror("Create socket");
        exit(FAILED_ERROR);
    }

    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = inet_addr(SERVER_ADDR);
    saddr.sin_port = htons(SERVER_PORT);

    if(connect(sockfd, (struct sockaddr*)&saddr, sizeof(saddr)) != 0) 
    {
        perror("connect");
        exit(FAILED_ERROR);
    }
    char *buff;
    buff = calloc(MTU_SIZE, 1);

    int read_len = MTU_SIZE;
    int readed_len = 0;
    int i = 0;
    int fd[5] = {-1};
    char* file_path;
    file_path = (char*)calloc(70, 1);
    unsigned int file_size;

    while(1)
    {
        if(i == 15)
        {
            break;
        }
        sprintf(file_path, "/home/ngoquang/Desktop/workplaces/Learn_CV/demo/hello%d.png", i);
        fd[i] = open(file_path, O_CREAT|O_WRONLY|O_APPEND, S_IRUSR|S_IWUSR);
        printf("file %d\n", i);
        read(sockfd, buff, 2*sizeof(int));
        memcpy(&file_size, buff, sizeof(unsigned int));
        printf("file size: %d\n", file_size);
        readed_len = 0;
        while(readed_len < file_size)
        {
            bzero(buff, MTU_SIZE);
            if(MTU_SIZE > (file_size - readed_len))
            {

                read_len = read(sockfd, buff, (file_size-readed_len));
            }
            else{
                read_len = read(sockfd, buff, MTU_SIZE);
            }
            readed_len += read_len;
            printf("readed: %d\n", readed_len);
            write(fd[i], buff, read_len);
        }
        close(fd[i]);
         
        i++;
    }
    free(buff);
    free(file_path);
    close(fd[i]);
    close(sockfd);
    return 0;
}