import numpy as np
import cv2

width, height = 640, 480
with open("./frame.raw", "rb") as f:
    raw = np.frombuffer(f.read(), dtype=np.uint8)

# Reshape into YUYV format
yuyv = raw.reshape((height, width, 2))
# Convert to BGR for display
bgr = cv2.cvtColor(yuyv, cv2.COLOR_YUV2BGR_YUYV)

cv2.imshow("frame", bgr)
cv2.waitKey(0)
