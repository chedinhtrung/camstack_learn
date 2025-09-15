#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

int main() {

    // open the device
    int fd = open("/dev/video0", O_RDWR);
    if (fd < 0) { perror("open"); return 1; }

    // Set format (640x480 YUYV for example)
    struct v4l2_format fmt = {0};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = 640;
    fmt.fmt.pix.height = 480;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;
    if (ioctl(fd, VIDIOC_S_FMT, &fmt) < 0) {
        perror("VIDIOC_S_FMT");
        return 1;
    }

    size_t sz = fmt.fmt.pix.sizeimage;
    void *buf = malloc(sz);

    // Grab one frame
    ssize_t r = read(fd, buf, sz);
    if (r < 0) { perror("read"); return 1; }

    // Write into a file
    FILE *f = fopen("frame.raw", "wb");
    fwrite(buf, 1, r, f);
    fclose(f);

    printf("Captured one frame (%zd bytes) to frame.raw\n", r);

    free(buf);
    close(fd);
    return 0;
}
