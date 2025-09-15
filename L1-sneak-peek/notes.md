- v4l-ctl is a tool that allows the interaction with the camera stack like a black box
1. Install it (if not already)
    
    ```jsx
    sudo apt install v4l-utils
    ```
    
2. List devices
    
    ```jsx
    v4l2-ctl --list-devices
    ```
    
3. See capabilities
    
    ```jsx
    v4l2-ctl --device=/dev/video0 --all
    ```
    
    results look like:
        
        Driver Info:
        Driver name      : uvcvideo
        Card type        : Integrated_Webcam_HD: Integrate
        Bus info         : usb-0000:00:14.0-5
        Driver version   : 6.8.12
        Capabilities     : 0x84a00001
        Video Capture
        Metadata Capture
        Streaming
        Extended Pix Format
        Device Capabilities
        Device Caps      : 0x04200001
        Video Capture
        Streaming
        Extended Pix Format
        Media Driver Info:
        Driver name      : uvcvideo
        Model            : Integrated_Webcam_HD: Integrate
        Serial           :
        Bus info         : usb-0000:00:14.0-5
        Media version    : 6.8.12
        Hardware revision: 0x00003805 (14341)
        Driver version   : 6.8.12
        Interface Info:
        ID               : 0x03000002
        Type             : V4L Video
        Entity Info:
        ID               : 0x00000001 (1)
        Name             : Integrated_Webcam_HD: Integrate
        Function         : V4L2 I/O
        Flags            : default
        Pad 0x01000007   : 0: Sink
        Link 0x02000013: from remote pad 0x100000a of entity 'Extension 4' (Video Pixel Formatter): Data, Enabled, Immutable
        Priority: 2
        Video input : 0 (Camera 1: ok)
        Format Video Capture:
        Width/Height      : 640/480
        Pixel Format      : 'YUYV' (YUYV 4:2:2)
        Field             : None
        Bytes per Line    : 1280
        Size Image        : 614400
        Colorspace        : sRGB
        Transfer Function : Rec. 709
        YCbCr/HSV Encoding: ITU-R 601
        Quantization      : Default (maps to Limited Range)
        Flags             :
        Crop Capability Video Capture:
        Bounds      : Left 0, Top 0, Width 640, Height 480
        Default     : Left 0, Top 0, Width 640, Height 480
        Pixel Aspect: 1/1
        Selection Video Capture: crop_default, Left 0, Top 0, Width 640, Height 480, Flags:
        Selection Video Capture: crop_bounds, Left 0, Top 0, Width 640, Height 480, Flags:
        Streaming Parameters Video Capture:
        Capabilities     : timeperframe
        Frames per second: 30.000 (30/1)
        Read buffers     : 0
        
        User Controls
        
        ```
                         brightness 0x00980900 (int)    : min=-64 max=64 step=1 default=0 value=0
                           contrast 0x00980901 (int)    : min=0 max=95 step=1 default=0 value=0
                         saturation 0x00980902 (int)    : min=0 max=100 step=1 default=64 value=64
                                hue 0x00980903 (int)    : min=-2000 max=2000 step=1 default=0 value=0
            white_balance_automatic 0x0098090c (bool)   : default=1 value=1
                              gamma 0x00980910 (int)    : min=100 max=300 step=1 default=100 value=100
               power_line_frequency 0x00980918 (menu)   : min=0 max=2 default=2 value=2 (60 Hz)
        			0: Disabled
        			1: 50 Hz
        			2: 60 Hz
          white_balance_temperature 0x0098091a (int)    : min=2800 max=6500 step=1 default=4600 value=4600 flags=inactive
                          sharpness 0x0098091b (int)    : min=1 max=7 step=1 default=2 value=2
             backlight_compensation 0x0098091c (int)    : min=0 max=3 step=1 default=3 value=3
        
        ```
        
        Camera Controls
        
        ```
                      auto_exposure 0x009a0901 (menu)   : min=0 max=3 default=3 value=3 (Aperture Priority Mode)
        			1: Manual Mode
        			3: Aperture Priority Mode
             exposure_time_absolute 0x009a0902 (int)    : min=3 max=2047 step=1 default=384 value=384 flags=inactive
        
        ```
        
    
    Notable: each control parameter (like brightness) has an address-like 32 bit number that is its ID, e.g `brightness 0x00980900 (int)    : min=-64 max=64 step=1 default=0 value=0` 
    
    This is what allows us to eventually do “hey driver, set the parameter whose ID is  `0x00980900`  to 10”
    
4. Capture a raw image
    
    ```jsx
    v4l2-ctl --device=/dev/video0 --stream-mmap --stream-count=1 --stream-to=frame.raw
    ```
    
    Py code to view it in the file ``view_raw.py``