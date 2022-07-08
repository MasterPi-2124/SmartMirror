#include <stdio.h>
#include <stdlib.h>
#include <linux/videodev2.h>
#include <sys/mman.h>
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
#include <sys/ioctl.h>

#include "fetch_img.h"

uint8_t *img_buff;
int     camFd;

#define TIMEOUT        (5)


static int xioctl(int fd, int request, void *arg)
{
        int r;
 
        do r = ioctl (fd, request, arg);
        while (-1 == r && EINTR == errno);
 
        return r;
}

int print_caps(int fd)
{
        struct v4l2_capability caps = {};
        if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &caps))
        {
                perror("Querying Capabilities");
                return 1;
        }
 
        printf( "Driver Caps:\n"
                "  Driver: \"%s\"\n"
                "  Card: \"%s\"\n"
                "  Bus: \"%s\"\n"
                "  Version: %d.%d\n"
                "  Capabilities: %08x\n",
                caps.driver,
                caps.card,
                caps.bus_info,
                (caps.version>>16)&&0xff,
                (caps.version>>24)&&0xff,
                caps.capabilities);
 
 
        // struct v4l2_cropcap cropcap = {0};
        // cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        // if (-1 == xioctl (fd, VIDIOC_CROPCAP, &cropcap))
        // {
        //         perror("Querying Cropping Capabilities");
        //         return 1;
        // }
 
        // printf( "Camera Cropping:\n"
        //         "  Bounds: %dx%d+%d+%d\n"
        //         "  Default: %dx%d+%d+%d\n"
        //         "  Aspect: %d/%d\n",
        //         cropcap.bounds.width, cropcap.bounds.height, cropcap.bounds.left, cropcap.bounds.top,
        //         cropcap.defrect.width, cropcap.defrect.height, cropcap.defrect.left, cropcap.defrect.top,
        //         cropcap.pixelaspect.numerator, cropcap.pixelaspect.denominator);
 
        int support_grbg10 = 0;
 
        // struct v4l2_fmtdesc fmtdesc = {0};
        // fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        char fourcc[5] = {0};
        // char c, e;
        // printf("  FMT : CE Desc\n--------------------\n");
        // while (0 == xioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc))
        // {
        //         strncpy(fourcc, (char *)&fmtdesc.pixelformat, 4);
        //         if (fmtdesc.pixelformat == V4L2_PIX_FMT_SGRBG10)
        //             support_grbg10 = 1;
        //         c = fmtdesc.flags & 1? 'C' : ' ';
        //         e = fmtdesc.flags & 2? 'E' : ' ';
        //         printf("  %s: %c%c %s\n", fourcc, c, e, fmtdesc.description);
        //         fmtdesc.index++;
        // }
        /*
        if (!support_grbg10)
        {
            printf("Doesn't support GRBG10.\n");
            return 1;
        }*/
 
        struct v4l2_format fmt = {0};
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width = 640;
        fmt.fmt.pix.height = 480;
        //fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_BGR24;
        //fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_GREY;
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_JPEG;
        fmt.fmt.pix.field = V4L2_FIELD_NONE;

        if (-1 == xioctl(fd, VIDIOC_G_FMT, &fmt))
        {
            perror("Setting Pixel Format");
            return 1;
        }
        if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt))
        {
            perror("Setting Pixel Format");
            return 1;
        }
 
        strncpy(fourcc, (char *)&fmt.fmt.pix.pixelformat, 4);
        printf( "Selected Camera Mode:\n"
                "  Width: %d\n"
                "  Height: %d\n"
                "  PixFmt: %s\n"
                "  Field: %d\n",
                fmt.fmt.pix.width,
                fmt.fmt.pix.height,
                fourcc,
                fmt.fmt.pix.field);
        return 0;
}
 
int init_mmap(int fd)
{
    struct v4l2_requestbuffers req = {0};
    req.count = 1;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
 
    if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req))
    {
        perror("Requesting Buffer");
        return 1;
    }
 
    struct v4l2_buffer buf = {0};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = 0;
    if(-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf))
    {
        perror("Querying Buffer");
        return 1;
    }
 
    img_buff = (uint8_t*)mmap (NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
    //S_BUFFER_LEN = buf.length;
    printf("Length: %d\nAddress: %p\n", buf.length, img_buff);
    printf("Image Length: %d\n", buf.bytesused);
 
    return 0;
}
 
int capture_image(int fd)
{
    struct v4l2_buffer buf = {0};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = 0;
    if(-1 == xioctl(fd, VIDIOC_QBUF, &buf))
    {
        perror("Query Buffer");
        return 1;
    }
 
    if(-1 == xioctl(fd, VIDIOC_STREAMON, &buf.type))
    {
        perror("Start Capture");
        return 1;
    }
 
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    struct timeval tv = {0};
    tv.tv_sec = 10;
    int r = select(fd+1, &fds, NULL, NULL, &tv);
    if(-1 == r)
    {
        perror("Waiting for Frame");
        return 1;
    }
 
    if(-1 == xioctl(fd, VIDIOC_DQBUF, &buf))
    {
        perror("Retrieving Frame");
        return 1;
    }
    printf ("sending image\n");
    printf("buffer len: %d\n", buf.length);
    printf("bytes used: %d\n", buf.bytesused);

    return buf.bytesused;
}


void handle_func()
{
    char* output_file_path = (char*)calloc(200, 1);
    int i = 0;
    int fd, bytesused;

    for (i = 1; i <= MAX_IMG_NUM; i++) {
        sprintf(output_file_path, "%s/hello%d.png", IMGS_FOLDER,i);
        fd = open(output_file_path, O_CREAT | O_WRONLY, 0666);
        if(fd == -1) break;
        bytesused = capture_image(camFd);
        write(fd, img_buff, bytesused);
        usleep(400000);
        close(fd);
    }
    
    free(output_file_path);
}


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
    camFd = open("/dev/video0", O_RDWR);
    printf("open returned %d: %s\n", camFd, strerror(errno));
    if(camFd == -1) return 1;
    if(print_caps(camFd)) return 1;
    if(init_mmap(camFd)) return 1;

    // pir_thread_func();
    handle_func();

    free(img_buff);
    close(camFd);
    return 0;
}

