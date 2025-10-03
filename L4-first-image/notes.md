- Previously: open device, query capabilities and pixel formats, setting a desired pixel format

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

- Workflow:
    1. **Configure the camera**. Use `ioctl` along with the respective ID of the requests to set parameters - most important is the *pixel format*.

    2. **Request buffers**. This is essentially telling the camera: When you capture frames, please put them into, say, 5 slots in the memory region. Caveat: the camera stack might decide to give us less than that! Where are those memory regions? How big are they? We'll only find out after we *query* the buffer. 

    3. For each buffer we have to **memory map** it into user space. Essentially, the memory region that we just got allocated in the previous step lives in *kernel* space - that we cannot directly read out. The `mmap()` operation can map that memory region into *user space* and returns a pointer that we can use to access the underlying kernel space memory. 
    To save the info of all the buffers, we create a struct called `buffer` (see code), call `mmap`for each, and store the info to an array of `buffer` structs. 

    4. **Queue all of the buffers**. At the beginning, we have to tell the camera stack: all these buffers are empty and ready to receive frame data into. THis is done by an `ioctl`call.

    4. **Tell the camera to start capturing frames**. This is done by sending an `ioctl` with the macro `VIDIOC_STREAMON`. At this point, the camera keeps recording frames and put them into the buffers that has been allocated in step 2. 

    5. In a loop, we **dequeue and requeue buffers continuously** - again, with an `ioctl` with the macro `VIDIOC_DQBUF`. This is asking the camera: are there any buffers that are filled with data? If there is, the camera returns the buffer. If not, the call blocks until a buffer is available. 

    In this context, the only thing we are interested in is the buffer's index, because in step 3 we saved the infos like the pointer to access the buffer and the size of the buffer in our array already. We use the pointer to read out the data and do whatever we want with it (send it, compress it, write to files, etc.). 
    
    Then we have to **queue the buffer back*. This means telling the camera stack: I am done with this memory region. You can fill it with new data now.

    6. After we are done, we have to tell the camera to stop capturing.

#### Code explanation (stream.c)

- What is `xioctl()`? This is a pattern in Linux called "retry on EINTR" (retry when error interrupted). Essentially, some events could interrupt our ioctl out of the blue by the system, and `ioctl` will "crash" and set `errno = EINTR` (to be explored later). We want in that case to retry the `ioctl` call - because this does not mean there is something wrong with the `ioctl` call - just that some other event interrupted it. 

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
