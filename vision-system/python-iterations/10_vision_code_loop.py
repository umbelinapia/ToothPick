import os
import cv2
import numpy as np
import requests
import time


# convert coordinates
def camera_to_robot(xc, yc):
    xr = 3900.0 + (xc - 50.0) * (9500.0 - 3900.0) / (250.0 - 50.0)
    yr = 0.0 + (yc - 20.0) * (7100.0 - 0.0) / (200.0 - 20.0)
    return int(round(xr)), int(round(yr))


# get image from ESP32
ESP32_URL = "http://172.20.10.2/capture"
IMAGE_PATH = "board.jpg"


while True:
    try:
        print("Capturing image...")

        response = requests.get(ESP32_URL, timeout=5)
        response.raise_for_status()

        with open(IMAGE_PATH, "wb") as f:
            f.write(response.content)

        # read image
        if not os.path.isfile(IMAGE_PATH):
            raise FileNotFoundError(f"Image not found: {IMAGE_PATH}")

        im = cv2.imread(IMAGE_PATH, cv2.IMREAD_GRAYSCALE)
        if im is None:
            raise RuntimeError(f"cv2.imread failed to load the image: {IMAGE_PATH}")

        # process image
        im_blur = cv2.GaussianBlur(im, (9, 9), 2)

        # detect cylinder
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

        # output image
        im_color = cv2.cvtColor(im, cv2.COLOR_GRAY2BGR)

        if circles is not None:
            circles = np.uint16(np.around(circles))

            if len(circles[0]) == 1:
                x, y, r = circles[0][0]

                cv2.circle(im_color, (x, y), r, (0, 0, 255), 1)

                xr, yr = camera_to_robot(x, y)

                print(f"Camera: x={x}, y={y}, r={r}")
                print(f"Robot:  x={xr}, y={yr}")
                print("Circle detected. Proceeding with robot action.")

                # optional: save final detection image
                cv2.imwrite("detected_circle.png", im_color)

                # break

            else:
                print(f"{len(circles[0])} circles detected. Waiting...")

        else:
            print("No circles detected. Waiting...")

    except requests.RequestException as e:
        print(f"Failed to capture image from ESP32: {e}")

    except Exception as e:
        print(f"Error: {e}")

    time.sleep(1)   # wait 1 second before trying again


print("Loop ended.")