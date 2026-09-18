# Standard imports
import os
import cv2
import numpy as np

import requests
from datetime import datetime
from pathlib import Path
import serial
import time

# convert coordinates
def camera_to_robot(xc, yc):

    print(f"Original camera coordinates: x={xc}, y={yc}")
    xc = max(80, min(280, xc))
    yc = max(40, min(200, yc))

    xr = 3900.0 + (xc - 50.0) * (9500.0 - 3900.0) / (280.0 - 80.0)
    yr = 0.0 + (yc - 40.0) * (7100.0 - 0.0) / (200.0 - 40.0)

    print(f"Mapped robot coordinates: x={int(xr)}, y={int(yr)}")

    return int(round(xr)), int(round(yr))

# get image from ESP32
ESP32_URL = "http://172.20.10.2/capture"

response = requests.get(ESP32_URL)
with open("board.jpg", "wb") as f:
    f.write(response.content)

# read image
path = "board.jpg" 
if not os.path.isfile(path):
    raise FileNotFoundError(f"Image not found: {path}")

im = cv2.imread(path, cv2.IMREAD_GRAYSCALE)
if im is None:
    raise RuntimeError(f"cv2.imread failed to load the image: {path}")

# process image 
# filter: grayscale median blur 
im_blur = cv2.GaussianBlur(im, (9, 9), 2)

# detect cylinder
# filter: hough circles detection
circles = cv2.HoughCircles(
    im_blur,            # the blurred grayscale input image
    cv2.HOUGH_GRADIENT, # detection method
    dp=1.2,
    minDist=80,         # minimum distance between the centers of detected circles.
    param1=100,         # edge detector threshold
    param2=20,          # accumulator threshold 
    minRadius=10,       
    maxRadius=20,      
)

# output
im_color = cv2.cvtColor(im, cv2.COLOR_GRAY2BGR)

if circles is not None:
    
    circles = np.uint16(np.around(circles))
    
    if len(circles[0]) == 1:
        x, y, r = circles[0][0]
        
        cv2.circle(im_color, (x, y), r, (0, 0, 255), 1) 

        xr, yr = camera_to_robot(x, y)

        # print(f"Camera: x={x}, y={y}, r={r}")
        # print(f"Robot:  x={xr}, y={yr}")

        # print("Proceeding with robot action")

    else:
        print(f"{len(circles[0])} circles detected")
    
else:
    print("no circles detected")

# hough_out = "hough_circles.png"
# cv2.imwrite(hough_out, im_color)
