import os
import cv2
import numpy as np
import requests
import time
import serial

ESP32_URL = "http://172.20.10.2/capture"
IMAGE_PATH = "board.jpg"

SERIAL_PORT = "COM7"
BAUD_RATE = 115200

TIMEOUT = 20

def camera_to_robot(xc, yc):
    xr = 3500.0 + (xc - 50.0) * (9500.0 - 3900.0) / (250.0 - 50.0)
    yr = 0.0 + (yc - 20.0) * (7100.0 - 0.0) / (200.0 - 20.0)
    return int(round(xr)), int(round(yr))


def wait_for_done(arduino, timeout=TIMEOUT):
    start_time = time.time()

    while time.time() - start_time < timeout:
        if arduino.in_waiting > 0:
            line = arduino.readline().decode(errors="ignore").strip()
            if line:
                print(f"Arduino: {line}")
            if line == "DONE":
                return True

        time.sleep(0.05)

    raise TimeoutError("Timed out waiting for Arduino DONE response")


def send_command(arduino, cmd):
    full_cmd = f"{cmd}\n"
    arduino.write(full_cmd.encode())
    # print(f"Sent to Arduino: {cmd}")
    wait_for_done(arduino)


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

    im_color = cv2.cvtColor(im, cv2.COLOR_GRAY2BGR)

    if circles is None:
        return None, im_color

    circles = np.uint16(np.around(circles))

    if len(circles[0]) != 1:
        return None, im_color

    x, y, r = circles[0][0]
    cv2.circle(im_color, (x, y), r, (0, 0, 255), 1)

    return (x, y, r), im_color


arduino = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
time.sleep(2)

# clear any old serial data
arduino.reset_input_buffer()

# startup sequence: wait for each move to finish before continuing
send_command(arduino, "home(J1)")
send_command(arduino, "home(J2)")
send_command(arduino, "home(J3)")
send_command(arduino, "cameraPose()")
send_command(arduino, "flip(180)")

print("Robot is in camera position. Starting vision loop...")

while True:
    try:
        print("Capturing image...")

        im = capture_image()
        result, im_color = detect_single_circle(im)

        if result is None:
            print("No single valid circle detected. Waiting...")
            time.sleep(1)
            continue

        x, y, r = result
        xr, yr = camera_to_robot(x, y)

        print(f"Camera: x={x}, y={y}, r={r}")
        print(f"Robot:  x={xr}, y={yr}")

        cv2.imwrite("detected_circle.png", im_color)

        send_command(arduino, f"coords(J1,25,{xr})")
        send_command(arduino, f"coords(J2,25,{yr})")
        send_command(arduino, "open()")
        send_command(arduino, "coords(J3,10,625)")
        send_command(arduino, "close()")
        send_command(arduino, "home(J3)")
        send_command(arduino, "flip(0)")
        send_command(arduino, "coords(J1,50,400)")
        send_command(arduino, "coords(J2,50,10150)")
        send_command(arduino, "open()")

        print("Cylinder delivered.")
        
        send_command(arduino, "cameraPose()")
        send_command(arduino, "flip(180)")
        
        print("Returning to vision loop.")

        time.sleep(2)
        continue

    except requests.RequestException as e:
        print(f"Failed to capture image from ESP32: {e}")

    except Exception as e:
        print(f"Error: {e}")

    time.sleep(1)

print("Loop ended.")
arduino.close()