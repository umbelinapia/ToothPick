import os
import cv2
import numpy as np
import requests
import time
import serial

# get image from ESP32
ESP32_URL = "http://172.20.10.2/capture"
IMAGE_PATH = "board.jpg"

# convert coordinates
def camera_to_robot(xc, yc):
    #xr = 3500.0 + (xc - 50.0) * (9500.0 - 3900.0) / (250.0 - 50.0) #robot x max - robot x min / camera x max - camera x min
    #yr = 0.0 + (yc - 20.0) * (7100.0 - 0.0) / (200.0 - 20.0) #robot y max - robot y min / camera y max - camera y min
    xr = 3500.0 + (xc - 50.0) * (9500.0 - 3900.0) / (250.0 - 30.0) #robot x max - robot x min / camera x max - camera x min
    yr = 0.0 + (yc - 20.0) * (7100.0 - 0.0) / (200.0 - 15.0) #robot y max - robot y min / camera y max - camera y min
    return int(round(xr)), int(round(yr))

def capture_image():
    response = requests.get(ESP32_URL, timeout=5)
    response.raise_for_status()

    with open(IMAGE_PATH, "wb") as f:
        f.write(response.content)

    if not os.path.isfile(IMAGE_PATH):
        raise FileNotFoundError(f"Image not found: {IMAGE_PATH}")

    im = cv2.imread(IMAGE_PATH, cv2.IMREAD_GRAYSCALE)
    if im is None:
        raise RuntimeError(f"cv2.imread failed to load image: {IMAGE_PATH}")

    return im

def detect_single_circle(im):
    im_blur = cv2.GaussianBlur(im, (9, 9), 2)

    circles = cv2.HoughCircles(
        im_blur,
        cv2.HOUGH_GRADIENT,
        dp=1.2,
        minDist=80,
        param1=100,
        param2=20,
        minRadius=10,
        maxRadius=20,
    )

im = capture_image()
result, im_color = detect_single_circle(im)
    
x, y, r = result
xr, yr = camera_to_robot(x, y)

print(f"Camera: x={x}, y={y}, r={r}")
print(f"Robot:  x={xr}, y={yr}")
