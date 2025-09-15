#### Open a device in C code

- All devices on Linux are file nodes. In our case, the camera is a thing that looks like `dev/video0`

- The method `open()` in C takes a file path (a `const char*`) and opens it, returning a simple `int` called a *file descriptor*. 

- We can use `ioctl()` on these file descriptors to issue commands to the hardware

Check the open.c file. To compile it do 
```
gcc open.c -o open
```
then run it.

Results: 
```
Driver: uvcvideo
Card: Integrated_Webcam_HD: Integrate
Bus: usb-0000:00:14.0-5
Caps: 0x84a00001
```

#### Notes on `ioctl` and related structs

- `ioctl` (IO-control) is part of the standard C library `sys/ioctl.h`
- docs: https://man7.org/linux/man-pages/man2/ioctl.2.html

- Common usage: 
```
ioctl(int fd, unsigned long request, ...)
```
where `fd` is a file descriptor, `request` is like an "opcode" (usually given by macros in the respective libraries, e.g `VIDIOC_QUERYCAP`), and ... is usually a *pointer* to a struct that will be filled in by the function. 

The ... part depends on the request code. Each code expects a different thing (specified in the docs). For example, `VIDIOC_QUERYCAP` expects a struct of type `v4l2_capability` that is defined in `linux/videodev2.h`

#### The struct `v4l2_capabilities`
- docs: https://docs.huihoo.com/doxygen/linux/kernel/3.7/structv4l2__capability.html
