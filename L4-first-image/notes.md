- Previoously: open device, query capabilities and pixel formats, setting a desired pixel format

- This lesson: capture a full image from the device. 

- 2 Methods. Most rudimentary: `read()` from the device. But this is not optimal if we want to *stream*, that is, output images continuously for, say, streaming video, because the bytes has been copied from kernel into user space. Also NOT EVERY PLATFORM SUPPORTS THIS `read()` CALL AND WILL THROW AN ERROR IN THAT CASE.


- Instead, the proper way is to *allocate* memory buffers in kernel space, then `mmap` (memory map) it into user space - so that we directly access the data spit out by the camera with no copying. 

- Mental picture: Like a conveyer belt of buckets to be filled with data, get out data from bucket, and put bucket back onto conveyer belt.  
    - Ask the camera driver "hey, give me a region of memory". This is called *requesting buffers*. 
    - Map into user space - "hey, allow me access to this memory by giving a pointer in user space"
    - `QBUF`: "hey, this buffer is ready to be filled. Fill them with data when you want."
    - `STREAMON`: Start continuous capture into this *buffer queue* - like a conveyer belt
    - `DQBUF`: get data from a filled buffer
    - `STREAMOFF`: Stop the conveyer belt

#### Code explanation (stream.c)

- What is `xioctl()`? This is a pattern in Linux called "retry on EINTR" (retry when error interrupted). Essentially, some events could interrupt our ioctl out of the blue by the system, and `ioctl` will "crash" and set `errno = EINTR` (to be explored later). We want in that case to retry the `ioctl` call. 

- To tell the driver "hey give me N buffers", we fill a struct `v4l2_requestbuffers`, then send it like a request using `xioctl`. 

- Driver then allocate memory regions. Here's the catch: The driver code chooses how big each ONE of the buffer is (buffers could have different sizes!), and assign each an index. It can even give you less, or even more, buffers than you asked for! It places the buffer *somewhere* in kernel memory that we cannot directly access, only `mmap` into user space. But first we have to know *where* each buffer is and *how big* is it - hence the next section:

- `ioctl` with `VIDIOC_QUERYBUF` and a filled `v4l2_buffer` is saying "Tell me about buffer i". We have to fill in the `index` field, amongs other stuff in the `v4l2_buffer` struct. The driver then fills the fields `b.length` and `b.m.offset` to indicate the size of each buffer and its location in memory. 

- After we know where each buffer is and how big it is, we `mmap` it - memory map it to user space. The call signature is 
```
void *mmap(void *addr, size_t length, int prot, int flags ,
           int fd, off_t offset);
```

doc: https://linux.die.net/man/2/mmap

which says: 
"Map the memory starting at `offset`, of size `length`, with protection level `prot`, flags `flags` into the current program's memory (heap)". `*addr` usually put to `NULL`, telling the kernel "you choose where to map". The result is a pointer to the mapped memory region, of type `void`. 

Under the hood, no copying happened. It just say "when I access pointer p returned from this mmap operation, the data is from `offset` and of size `length` instead.

- At the beginning, we `QBUF` (queue buffer) all of the buffers to be filled with data. Then we do `xioctl(fd, VIDIOC_DQBUF, &b)` for a buffer `v4l2_buffer b` (that we also have to fill), to say "Give me a filled buffer when there is any". 

- To read its content, we look at the saved info in our array "buffer[4]", and pick out the info (offset, length) of the buffer we were given (`b.index`). We then call `fwrite` to write to an opened file, passing in the offset and the size to be written.

- Finally we stop the stream with `xioctl(fd, VIDIOC_STREAMOFF, &type)`.
