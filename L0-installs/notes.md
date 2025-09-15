#### Install libraries and build tools

```
sudo apt update
sudo apt install libcamera0 libcamera-dev v4l-utils ffmpeg build-essential pkg-config
```
- `build-essentials`: contains compilers (gcc, g++)
- `libcamera-dev`: headers to write apps with libcamera
- `libcamera0`: the libcamera library
- `v4l-utils`: use this tool to discover camera devices and query their specifications, controls and parameters.
- `pkg-config`: a helper tool when building C/C++ on Linux. It tells us for a certain package: where are the headers located, what is the name of the library, are there flags etc.

#### Check that things are correctly installed
First, the library files (.so files)
```
ls -l /usr/lib/x86_64-linux-gnu/libcamera*.so*
```
The headers (should be a bunch of .h files)
```
ls /usr/include/libcamera/libcamera
```

using `pkg-config` (this is helpful when calling gcc to compile, pkg-config automatically resolves include paths and library paths for us)

```
pkg-config --modversion camera
```
should show a version (1.0)
