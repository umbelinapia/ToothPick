# Standard imports
import os
import cv2
import numpy as np
 
# Read image
path = "board.jpg"  # update to the right filename/path if needed
if not os.path.isfile(path):
    raise FileNotFoundError(f"Image not found: {path}")

im = cv2.imread(path, cv2.IMREAD_GRAYSCALE)
if im is None:
    raise RuntimeError(f"cv2.imread failed to load the image: {path}")

# Hough Circle detection 

# Applies Gaussian smoothing to reduce noise and small texture details.
# Hough circle detection is sensitive to noisy edges. Blurring helps by:
#   smoothing out tiny variations
#   making edge structure cleaner
#   reducing false circle detections

im_blur = cv2.GaussianBlur(im, (9, 9), 2)

circles = cv2.HoughCircles(
    im_blur,            # The blurred grayscale input image.
    cv2.HOUGH_GRADIENT, # detection method.
    dp=1.2,
    minDist=80,         # Minimum distance between the centers of detected circles.
    param1=100,         #  edge detector threshold
    param2=20,          # accumulator threshold 
    minRadius=10,       
    maxRadius=20,      
)

# Create a color version of the original so circles draw in red
im_color = cv2.cvtColor(im, cv2.COLOR_GRAY2BGR)

if circles is not None:
    circles = np.uint16(np.around(circles))
    for (x, y, r) in circles[0, :]:
        cv2.circle(im_color, (x, y), r, (0, 0, 255), 2)
        print('x =', x, 'y =', y, 'radius =', r)
else:
    print("no circles detected")

hough_out = "hough_circles.png"
cv2.imwrite(hough_out, im_color)
