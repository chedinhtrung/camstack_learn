- A *pixel format* defines how pixels are stored in memory, how color is represented, and how many bytes per pixel. 
    - Example: sRGB stores 8 bit per color, 3 values red-green-blue for each pixel
    - Example: YUYV stores 1 U and 1 V value for each *pair* of pixels, along with the lumniosity (Y) for each
    - Weirder formats: Bayer raw, H264, etc. 

- A camera could be capable of outputting multiple formats and outputs a format by default. This lesson queries the camera for that and prints out the supported formats. Also we can set the desired format and resolution via `ioctl`

- The FourCC (four character code) is the human readable code for the formats. e.g YUYV

- To query the camera for the pixel formats it supports, we do:
    - open the device 
    - call `ioctl` with the respective opcode: `VIDIOC_ENUM_FMT` and the required struct: `v4l2_fmtdesc`. This struct has an *index* field that we can loop through to see all of the capabilities of the camera.
    - Each `v4l2_fmtdesc` can have several frame sizes, frame rates, etc. that we can also query with the respective opcode and required structs and loop through with their index. For example, we can do `ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &fsz)` with the struct `v4l2_frmsizeenum` (that we have to partially fill first!) to get framesize of each pixel formats.

- To then set the desired pixel formats (they call it "negotiate"), we do:
    - Fill a struct `v4l2_format` with our desired parameters
    - Send it via `ioctl(fd, VIDIOC_S_FMT, &fmt)`

