#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

static void fourcc_to_str(__u32 f, char s[5]) {
    s[0] = f & 0xFF;
    s[1] = (f >> 8) & 0xFF;
    s[2] = (f >> 16) & 0xFF;
    s[3] = (f >> 24) & 0xFF;
    s[4] = 0;
}

int main(int argc, char** argv) {
    const char* dev = "/dev/video0";
    int fd = open(dev, O_RDWR);
    if (fd < 0) { perror("open"); return 1; }

    // 1) Enumerate pixel formats
    struct v4l2_fmtdesc fmtd = {0};
    fmtd.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    printf("=== Pixel formats ===\n");
    for (fmtd.index = 0; ioctl(fd, VIDIOC_ENUM_FMT, &fmtd) == 0; fmtd.index++) {
        char fourcc[5]; fourcc_to_str(fmtd.pixelformat, fourcc);
        printf("[%2u] %s (%s)\n", fmtd.index, fmtd.description, fourcc);

        // 2) For each format, enumerate supported frame sizes
        struct v4l2_frmsizeenum fsz = {0};
        fsz.pixel_format = fmtd.pixelformat;
        printf("     Sizes:");
        int printed_header = 0;
        for (fsz.index = 0; ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &fsz) == 0; fsz.index++) {
            if (fsz.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
                if (!printed_header) { printf("\n"); printed_header = 1; }
                printf("       %ux%u\n", fsz.discrete.width, fsz.discrete.height);

                // 3) For each size, enumerate frame intervals (fps)
                struct v4l2_frmivalenum fi = {0};
                fi.pixel_format = fmtd.pixelformat;
                fi.width  = fsz.discrete.width;
                fi.height = fsz.discrete.height;
                for (fi.index = 0; ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &fi) == 0; fi.index++) {
                    if (fi.type == V4L2_FRMIVAL_TYPE_DISCRETE) {
                        double fps = fi.discrete.denominator / (double)fi.discrete.numerator;
                        printf("         %.3f fps\n", fps);
                    }
                }
            } else {
                // Stepwise/continuous not handled here (keep lesson compact)
                if (!printed_header) { printf(" (stepwise/continuous)\n"); printed_header = 1; }
            }
        }
        if (!printed_header) printf(" (no size info)\n");
    }

    // 4) Pick a format (try 640x480 YUYV) and set it
    struct v4l2_format fmt = {0};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width        = 640;
    fmt.fmt.pix.height       = 480;
    fmt.fmt.pix.pixelformat  = V4L2_PIX_FMT_YUYV;   // change to V4L2_PIX_FMT_MJPEG etc. if you prefer
    fmt.fmt.pix.field        = V4L2_FIELD_NONE;

    if (ioctl(fd, VIDIOC_S_FMT, &fmt) < 0) {
        perror("VIDIOC_S_FMT");
        close(fd);
        return 1;
    }

    char chosen[5]; fourcc_to_str(fmt.fmt.pix.pixelformat, chosen);
    printf("\n=== Negotiated format ===\n");
    printf("Size: %ux%u  FourCC: %s  Bytes/line: %u  Image size: %u\n",
           fmt.fmt.pix.width, fmt.fmt.pix.height, chosen,
           fmt.fmt.pix.bytesperline, fmt.fmt.pix.sizeimage);

    close(fd);
    return 0;
}
