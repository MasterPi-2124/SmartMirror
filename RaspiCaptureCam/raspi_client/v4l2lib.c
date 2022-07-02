#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/mman.h>
#include <unistd.h>




/*
    L4V2 implementation
*/

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
        fmt.fmt.pix.width = 1280;
        fmt.fmt.pix.height = 960;
        //fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_BGR24;
        //fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_GREY;
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
        fmt.fmt.pix.field = V4L2_FIELD_NONE;
        
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
 
int init_mmap(int fd, uint8_t *buffer)
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
 
    buffer = (uint8_t*)mmap (NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
    //S_BUFFER_LEN = buf.length;
    printf("Length: %d\nAddress: %p\n", buf.length, buffer);
    printf("Image Length: %d\n", buf.bytesused);
 
    return 0;
}
 
static int i = 0;

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
    // int i = 0;
    // int write_len=0;
    // while(MTU_SIZE*i <= buf.bytesused)
    // {
    //     write_len += write(outfd, buffer+MTU_SIZE*i, MTU_SIZE);
    //     i++;
    // }
    // printf("writen: %d\n", write_len);
    // char* ouput_file_path;
    // ouput_file_path = (char*)calloc(20, 1);
    // sprintf(ouput_file_path, "/tmp/hello%d.png", i);
    // i++;
    // int img_fd = open(ouput_file_path, O_RDWR|O_CREAT , S_IWUSR|S_IRUSR);
    // write(img_fd, buffer, buf.bytesused);
    // free(ouput_file_path);
    // close(img_fd);

    return 0;
}

int init_v4l2(int fd, uint8_t *buf)
{
    fd = open("/dev/video0", O_RDWR);
    printf("open return %d: %s\n", fd, strerror(errno));
    if(print_caps(fd)) return -1;
    if(init_mmap(fd, buf)) return -1;
    return 0;
}
