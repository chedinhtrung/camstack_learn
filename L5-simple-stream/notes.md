
- By continuously `QBUF` and `DQBUF` we can *stream* - basically just keep processing one frame after another. 

The file `stream.c` is almost identical to the one in L4. Instead of dequeueing the buffer once, we simple dequeue it, write its content to a file, then queue the buffer again: 

```
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
```
Also note that since the video device can output MJPEG, we set its pixel format to MJPEG and saved to a `.mjpeg` file, which can be played by using `ffplay`: 

```
ffplay -f mjpeg -i video.mjpeg
```
