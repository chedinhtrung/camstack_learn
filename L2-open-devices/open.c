#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

int main(int argc, char** argv){
    
    int fd = open("/dev/video0", O_RDWR);
    
    if (fd < 0) {perror("Cannot open"); return 1;}

    struct v4l2_capability cap;
    if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0){
        perror("Bad device");
        return 1;
    }
    printf("Driver: %s\n", cap.driver);
    printf("Card: %s\n", cap.card);

    printf("Bus: %s\n", cap.bus_info);
    printf("Caps: 0x%x\n", cap.capabilities);

    return 0;
}