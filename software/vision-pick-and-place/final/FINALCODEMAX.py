import time
import serial

# Standard imports
import os
import cv2
import numpy as np

import requests
from datetime import datetime
from pathlib import Path

# convert coordinates
def camera_to_robot(xc, yc):
    xc = float(xc)
    yc = float(yc)

    # clamp camera values
    xc = max(30.0, min(250.0, xc))
    yc = max(30.0, min(200.0, yc))

    print(f"Clamped camera coordinates: x={xc}, y={yc}")

    xr = 2500.0 + (xc - 30.0) * 4500.0 / 220.0
    yr = (yc - 30.0) * 7000.0 / 170.0
    
    print(f"Mapped robot coordinates: x={int(round(xr))}, y={int(round(yr))}")

    # deduction
    t = (200.0 - yc) / 185.0
    y_deduction = 1000.0 * (t ** 0.8)

    yr -= y_deduction

    #y_deduction = 250.0 * (200.0 - yc) / 185.0
    #yr = yr - 250.0
    
    yr = max(0.0, min(7000.0, yr))
    print(f"Mapped deducted robot coordinates: x={int(round(xr))}, y={int(round(yr))}")

    return int(round(xr)), int(round(yr))

# get image from ESP32
ESP32_URL = "http://172.20.10.2/capture"


# hough_out = "hough_circles.png"
# cv2.imwrite(hough_out, im_color)



SERIAL_PORT = "COM10"
BAUD_RATE = 115200
TIMEOUT = 40

# -----------------------------
# SPEEDS
# -----------------------------
J1_SPEED = 250
J2_SPEED = 300
J3_SPEED = 25

# -----------------------------
# IMPORTANT POSITIONS
# Change these easily later
# -----------------------------
J3_PICK_DOWN = 700
J3_DROP_HEIGHT = 100
J3_HOME_UP = 0

CAM_J1 = 4000
CAM_J2 = 7500
CAM_J3 = 0

DROP_J2 = 10150

# -----------------------------
# DELAYS
# -----------------------------
START_DELAY = 1.0
OPEN_DELAY = 1.0
DOWN_DELAY = 1.0
GRAB_DELAY = 1.0
LIFT_DELAY = 2.0
FLIP_DELAY = 1.5
PRE_DROP_DELAY = 1.0
DROP_DELAY = 1.0
POST_DROP_CLOSE_DELAY = 0.5
POST_DROP_LIFT_DELAY = 1.5
HOME_DELAY = 1.0
FINAL_DELAY = 1.0

MAX_X_ROBOT = 7150
EDGE_Y_THRESHOLD = 1000


def read_available_lines(arduino, duration=1.5):
    end_time = time.time() + duration
    while time.time() < end_time:
        while arduino.in_waiting > 0:
            line = arduino.readline().decode(errors="ignore").strip()
            if line:
                print(f"Arduino: {line}")
        time.sleep(0.02)


def wait_for_done(arduino, timeout=TIMEOUT):
    start_time = time.time()

    while time.time() - start_time < timeout:
        if arduino.in_waiting > 0:
            line = arduino.readline().decode(errors="ignore").strip()

            if line:
                print(f"Arduino: {line}")

            if line == "DONE":
                return True

            if line.startswith("ERROR"):
                raise RuntimeError(line)

        time.sleep(0.02)

    raise TimeoutError("Timed out waiting for Arduino DONE response")


def send_command(arduino, cmd, timeout=TIMEOUT):
    arduino.write((cmd + "\n").encode())
    arduino.flush()
    print(f"Sent: {cmd}")
    wait_for_done(arduino, timeout)


arduino = None

try:
    arduino = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    time.sleep(3)
    arduino.reset_input_buffer()
    arduino.reset_output_buffer()

    read_available_lines(arduino, duration=1.0)

    # -----------------------------
    # START SAFE / HOME
    # -----------------------------
    send_command(arduino, "close()")
    send_command(arduino, "home(J1)")
    send_command(arduino, "home(J2)")
    send_command(arduino, "home(J3)")

    # Flip is ONLY allowed at true J3 home
    send_command(arduino, "flipUp()")
    time.sleep(START_DELAY)

    # -----------------------------
    # MOVE TO CAMERA POSITION
    # -----------------------------
    send_command(arduino, f"coords(J1,{J1_SPEED},{CAM_J1})")
    send_command(arduino, f"coords(J2,{J2_SPEED},{CAM_J2})")
    send_command(arduino, f"coords(J3,{J3_SPEED},{CAM_J3})")

    print("At camera position, flip UP.")
    time.sleep(5.0)


    # -----------------------------
    # get cylinder location
    # -----------------------------
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

#    if circles is not None:
#
#        circles = np.uint16(np.around(circles))
#
#        if len(circles[0]) == 1:
#            x, y, r = circles[0][0]
#
#            cv2.circle(im_color, (x, y), r, (0, 0, 255), 1) 
#
#            xr, yr = camera_to_robot(x, y)
#
#            # print(f"Camera: x={x}, y={y}, r={r}")
#            # print(f"Robot:  x={xr}, y={yr}")
#
#            # print("Proceeding with robot action")
#
#        else:
#            print(f"{len(circles[0])} circles detected")
#
#    else:
#        print("no circles detected")

    use_edge_routine = False

    if circles is not None:

        circles = np.around(circles[0]).astype(int)

        if len(circles) == 1:
            x, y, r = circles[0]

            cv2.circle(im_color, (x, y), r, (0, 0, 255), 1)

            xr, yr = camera_to_robot(x, y)

            if yr < EDGE_Y_THRESHOLD:
                use_edge_routine = True
                print("Using edge approach routine because robot Y < 1000")

        else:
            raise RuntimeError(f"{len(circles[0])} circles detected; expected exactly 1")

    else:
        raise RuntimeError("No circles detected")
    

    # -----------------------------
    # MOVE TO PICK POSITION
    # -----------------------------
    if use_edge_routine:
        print("Edge routine start.")

        # Go to detected Y first
        send_command(arduino, f"coords(J2,{J2_SPEED},{yr})")

        # Then go to max X
        send_command(arduino, f"coords(J1,{J1_SPEED},{MAX_X_ROBOT})")

        print("At edge pre-pick position.")

        # Open at edge
        send_command(arduino, "opentight()")
        time.sleep(OPEN_DELAY)

        # Go down at edge
        send_command(arduino, f"coords(J3,{J3_SPEED},{J3_PICK_DOWN})")
        time.sleep(DOWN_DELAY)


        # Move across to detected X while still down
        send_command(arduino, f"coords(J1,{J1_SPEED},{xr})")

        print("Moved from edge to target X, continuing regular routine.")

    else:
        send_command(arduino, f"coords(J1,{J1_SPEED},{xr})")
        send_command(arduino, f"coords(J2,{J2_SPEED},{yr})")

        print("At pick position.")

        # Open only once at pick location
        send_command(arduino, "open()")
        time.sleep(OPEN_DELAY)

    #    # -----------------------------
    #    # MOVE TO PICK POSITION
    #    # -----------------------------
    #    send_command(arduino, f"coords(J1,{J1_SPEED},{xr})")
    #    send_command(arduino, f"coords(J2,{J2_SPEED},{yr})")
    #
    #    print("At pick position.")
    #
    #    # Open only once at pick location
    #    send_command(arduino, "open()")
    #    time.sleep(OPEN_DELAY)

    # -----------------------------
    # GO DOWN / PICK UP
    # -----------------------------
    send_command(arduino, f"coords(J3,{J3_SPEED},{J3_PICK_DOWN})")
    time.sleep(DOWN_DELAY)

    send_command(arduino, "close()")
    time.sleep(GRAB_DELAY)

    # -----------------------------
    # GO BACK TO TRUE J3 HOME
    # because flip is only allowed there
    # -----------------------------
    send_command(arduino, "home(J3)")
    time.sleep(LIFT_DELAY)

    # -----------------------------
    # FLIP DOWN AT TRUE J3 HOME
    # -----------------------------
    send_command(arduino, "flipDown()")
    time.sleep(FLIP_DELAY)

    # -----------------------------
    # IMPORTANT CHANGE:
    # KEEP J3 UP while J1 and J2 move to drop position
    # -----------------------------
    send_command(arduino, "home(J1)")
    send_command(arduino, f"coords(J2,{J2_SPEED},{DROP_J2})")
    send_command(arduino, "pos()")
    time.sleep(PRE_DROP_DELAY)

    # -----------------------------
    # ONLY NOW GO DOWN TO DROP HEIGHT
    # -----------------------------
    send_command(arduino, f"coords(J3,{J3_SPEED},{J3_DROP_HEIGHT})")
    time.sleep(DOWN_DELAY)

    # -----------------------------
    # DROP
    # -----------------------------
    send_command(arduino, "dropOpen()")
    time.sleep(DROP_DELAY)

    # -----------------------------
    # CLOSE AGAIN AFTER DROPPING
    # -----------------------------
    send_command(arduino, "close()")
    time.sleep(POST_DROP_CLOSE_DELAY)

    # -----------------------------
    # GO BACK UP BEFORE HOMING EVERYTHING
    # -----------------------------
    send_command(arduino, "home(J3)")
    time.sleep(POST_DROP_LIFT_DELAY)

    # -----------------------------
    # MOVE BACK HOME
    # -----------------------------
    send_command(arduino, "home(J1)")
    send_command(arduino, "home(J2)")
    send_command(arduino, "home(J3)")
    time.sleep(HOME_DELAY)

    # -----------------------------
    # END AT FLIP UP
    # flip only allowed at J3 home
    # -----------------------------
    send_command(arduino, "flipUp()")
    time.sleep(FINAL_DELAY)

    print("Cycle complete.")
    print("Sequence now is:")
    print("pick -> lift -> flip -> move J1/J2 to drop -> lower J3 -> drop -> close -> raise J3 -> home all")

except KeyboardInterrupt:
    print("Stopped by user.")

except Exception as e:
    print(f"Error: {e}")

finally:
    if arduino is not None and arduino.is_open:
        arduino.close()
        print("Serial port closed.")