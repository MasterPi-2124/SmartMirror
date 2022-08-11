#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <errno.h>
#include <unistd.h>
#include <time.h>

#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/fcntl.h>

#include "client.h"

#include "v4l2lib.h"

uint8_t *img_buff;
int     camFd;

#define TIMEOUT        (5)




void handle_func()
{
    int socket, n;
    char* buffer;
    buffer = (char*)calloc(BUFFER_SIZE, 1);
    time_t start  = time(NULL);
    time_t end    = start + TIMEOUT;
    time_t current;

    // wait to connect to server
    while(1)
    {
        current = time(NULL);
        if(current >= end){
            printf("connection timeout!\n");
            goto HANDLE_END;
        } 
        if(CL_SUCCESS == client_init(&socket)) break;
        usleep(50000);
    }
    printf("client_init() successfull!\n");
    
    // comunication
    // step 1: send request to server
    memset(buffer, 0, BUFFER_SIZE);
    strcpy(buffer, REQ_CMD);
    printf("SEND: %s\n", buffer);
    n = write(socket, buffer, strlen(buffer));
    printf("write returned %d: %s\n", n, strerror(errno));
    // step 2: recv ack from server
    memset(buffer, 0, BUFFER_SIZE);
    n = read(socket, buffer, BUFFER_SIZE);
    printf("read returned %d [Data: %s]: %s\n", n, buffer,strerror(errno));
    n = recv_cmd(buffer);
    if(n != 200) goto HANDLE_END;
    // int bytesused;
    // while(1)
    // {
    //     for(;;)
    //     {
    //         bytesused = capture_image(camFd);
    //         // step 3: send image capture to server
    //         while (1)
    //         {
    //             int size = send(socket, &img_buff, BUFFER_SIZE, 0);
    //         }
            
    //         // step 4: recv ack for each image from server
    //         memset(buffer, 0, BUFFER_SIZE);
    //         recv(socket, buffer, BUFFER_SIZE, 0);
    //         recv_cmd(buffer);

            

    //     }


    // }
    // step 5: recv response from server
    // end comunication
    // n = read(socket, buffer, BUFFER_SIZE);
    // printf("read returned %d: %s\n", n, strerror(errno));
    // n = recv_cmd(buffer);
    // printf("n = %d\n", n);
    free(buffer);

    if(CL_FAILED == client_deinit(&socket))
    {
        printf("client_deinit() failed\n");
    }
    printf("client_deinit() successfull!\n");

    HANDLE_END:
        printf("can't connect to server\n");
}

// int pir_thread_func()
// {
//     int inotifyFd, wd;
//     int curPIR;

//     // create inotify instance
//     inotifyFd = inotify_init();
//     if(inotifyFd == -1)
//     {
//         perror("inotify_init()");
//         return CL_FAILED;
//     }
//     // check modify only
//     wd = inotify_add_watch(inotifyFd, PIR_STATUS_FILE, IN_CLOSE_WRITE);
//     if(wd == -1)
//     {
//         perror("inotify_add_watch()");
//         return CL_FAILED;
//     }
//     char* buff;
//     buff = (char*)calloc(BUFFER_SIZE, 1);

//     // pir process
//     while (1)
//     {
//         if(read(inotifyFd, buff, BUFFER_SIZE))
//         {
//             curPIR = read_pir_status();
//             if(curPIR == 1)
//             {
//                 handle_func();
//             }
//         }
//     }
    

// }
/**
 * @brief to enable a gpio pin, set input or output
 * 
 * @param gpio_num 
 * @param direction 
 */
void export_gpio(int gpio_num, char direction)
{
    char *buf_cmd;
    buf_cmd = (char*)calloc(255, 1);

    sprintf(buf_cmd, "echo %d > /sys/class/gpio/export", gpio_num);
    system(buf_cmd);
    memset(buf_cmd, 0, 255);

    // set gpio is input
    if('i' == direction)
    {
        sprintf(buf_cmd, "echo in > /sys/class/gpio/gpio%d/direction", gpio_num);
    }
    // set gpio is output
    else if('o' == direction)
    {
        sprintf(buf_cmd, "echo out > /sys/class/gpio/gpio%d/direction", gpio_num);
    }
    system(buf_cmd);
    memset(buf_cmd, 0, 255);
    
    sprintf(buf_cmd, "echo both > /sys/class/gpio/gpio%d/edge", gpio_num);
    system(buf_cmd);

    free(buf_cmd);
}


void pir_thread_func(void)
{
    int n;
    int epfd = epoll_create(1);

    // to check gpio 
    n = access(PIR_STATUS_FILE, F_OK);
    printf("access returned %d: %s\n", n, strerror(errno));
    if(n != 0)
    {
        export_gpio(PIR_PIN, 'i');
    }

    int fd = open(PIR_STATUS_FILE, O_RDWR | O_NONBLOCK);
    printf("open returned %d: %s\n", fd, strerror(errno));
    if(fd > 0)
    {
        char buf = 0;
        struct epoll_event ev;
        struct epoll_event events;
        ev.events = EPOLLPRI|EPOLLERR;
        ev.data.fd = fd;

        n = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
        printf("epoll_ctl returned %d:%s\n", n, strerror(errno));

        while(1) {
            n = epoll_wait(epfd, &events, 1, -1);
            printf("epoll returned %d: %s\n", n, strerror(errno));

            if(n > 0)
            {
                n = lseek(fd, 0, SEEK_SET);
                printf("seek %d bytes: %s\n", n, strerror(errno));
                n = read(fd, &buf, 1);
                if('1' == buf)
                {
                    handle_func();
                    
                    // printf("read %d bytes: %s\n", n, strerror(errno));
                    // printf("buf = 0x%x\n", buf);
                } else {
                    printf("goodbye!\n");
                }
            }
        }
    }
}


int main(int argc, char* argv[])
{
    if (init_v4l2(camFd, img_buff) == -1) return 1;
    pir_thread_func();

    free(img_buff);
    close(camFd);
    return 0;
}

