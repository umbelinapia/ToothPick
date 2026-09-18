import numpy as np
import cv2
import os

path = "image.jpeg"  # or a full path like r"C:\path\to\image.jpg"

if not os.path.isfile(path):
    raise FileNotFoundError(f"Image not found: {path}")

img = cv2.imread(path, cv2.IMREAD_GRAYSCALE)
if img is None:
    raise RuntimeError(f"cv2.imread failed to load the image: {path}")

h, w = img.shape
...

# img = cv2.imread("image.jpeg", cv2.IMREAD_GRAYSCALE)

h, w = img.shape

threshold = 180

sumX = 0
sumY = 0
count = 0

minX, minY = w, h
maxX, maxY = 0, 0

for y in range(h):
    for x in range(w):
        if img[y, x] >= threshold:
            sumX += x
            sumY += y
            count += 1

            minX = min(minX, x)
            minY = min(minY, y)
            maxX = max(maxX, x)
            maxY = max(maxY, y)

if count > 50:
    cx = sumX / count
    cy = sumY / count
    radius = ((maxX - minX) + (maxY - minY)) / 4

    print("Detected:")
    print("cx =", cx)
    print("cy =", cy)
    print("radius =", radius)
else:
    print("No object detected")