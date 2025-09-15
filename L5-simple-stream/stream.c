
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>

#define NUM_BUFS 4

struct buffer {
    void  *start;
    size_t length;
};

static int xioctl(int fd, int req, void *arg) {
    int r;
    do { r = ioctl(fd, req, arg); } while (r == -1 && errno == EINTR);
    return r;
}

int main() {
    const char *dev = "/dev/video0";
    int fd = open(dev, O_RDWR);
    if (fd < 0) { perror("open"); return 1; }

    // 1) Set a format (driver may adjust it)
    struct v4l2_format fmt = {0};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width        = 640;
    fmt.fmt.pix.height       = 480;
    fmt.fmt.pix.pixelformat  = V4L2_PIX_FMT_MJPEG;   // try MJPEG if YUYV unsupported
    fmt.fmt.pix.field        = V4L2_FIELD_NONE;
    if (xioctl(fd, VIDIOC_S_FMT, &fmt) < 0) { perror("VIDIOC_S_FMT"); return 1; }

    // 2) Request MMAP buffers
    struct v4l2_requestbuffers req = {0};
    req.count  = NUM_BUFS;
    req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    if (xioctl(fd, VIDIOC_REQBUFS, &req) < 0) { perror("VIDIOC_REQBUFS"); return 1; }
    if (req.count < 2) { fprintf(stderr, "Not enough buffers\n"); return 1; }

    // 3) Query & mmap each buffer
    struct buffer bufs[NUM_BUFS] = {0};
    for (unsigned i = 0; i < req.count; i++) {
        struct v4l2_buffer b = {0};
        b.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        b.memory = V4L2_MEMORY_MMAP;
        b.index  = i;
        if (xioctl(fd, VIDIOC_QUERYBUF, &b) < 0) { perror("VIDIOC_QUERYBUF"); return 1; }
        bufs[i].length = b.length;
        bufs[i].start  = mmap(NULL, b.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, b.m.offset);
        if (bufs[i].start == MAP_FAILED) { perror("mmap"); return 1; }
    }

    // 4) Queue all buffers empty
    for (unsigned i = 0; i < req.count; i++) {
        struct v4l2_buffer b = {0};
        b.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        b.memory = V4L2_MEMORY_MMAP;
        b.index  = i;
        if (xioctl(fd, VIDIOC_QBUF, &b) < 0) { perror("VIDIOC_QBUF"); return 1; }
    }

    // 5) Start streaming
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (xioctl(fd, VIDIOC_STREAMON, &type) < 0) { perror("VIDIOC_STREAMON"); return 1; }

    // 6) setup the buffer, then dequeue it
    struct v4l2_buffer b = {0};
    b.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    b.memory = V4L2_MEMORY_MMAP;

    FILE *f = fopen("video.mjpeg", "wb");
    if (!f) { perror("fopen"); return 1; }

    for (int i=0; i<100; i++){

        // Dequeue a buffer, write its content to a file
        if (xioctl(fd, VIDIOC_DQBUF, &b) < 0) { perror("VIDIOC_DQBUF"); return 1; }
        fwrite(bufs[b.index].start, 1, b.bytesused ? b.bytesused : bufs[b.index].length, f);
        
        //Queue the buffer back
        if (xioctl(fd, VIDIOC_QBUF, &b) < 0) { perror("VIDIOC_QBUF (re)"); }
    }
    


    
    fclose(f);

    // 9) Stop streaming and clean up
    if (xioctl(fd, VIDIOC_STREAMOFF, &type) < 0) { perror("VIDIOC_STREAMOFF"); }
    for (unsigned i = 0; i < req.count; i++) munmap(bufs[i].start, bufs[i].length);
    close(fd);
    return 0;
}
