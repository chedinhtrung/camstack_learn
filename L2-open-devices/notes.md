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