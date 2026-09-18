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

def send_command(arduino, cmd):
    full_cmd = f"{cmd}\n"
    arduino.write(full_cmd.encode())
    # print(f"Sent to Arduino: {cmd}")
    wait_for_done(arduino)


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



arduino = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
time.sleep(2)

# clear any old serial data
arduino.reset_input_buffer()

# startup sequence: wait for each move to finish before continuing
send_command(arduino, "close()")

send_command(arduino, "home(J1)")

time.sleep(5)

send_command(arduino, "home(J2)")
time.sleep(5)

send_command(arduino, "home(J3)")
time.sleep(5)

send_command(arduino, "cameraPose()")
time.sleep(5)

send_command(arduino, "flip(180)")
time.sleep(5)

print("Robot is in camera position. Starting vision loop...")



send_command(arduino, "open()")

time.sleep(5)
send_command(arduino, "coords(J3,25,700)")

time.sleep(5)
send_command(arduino, "close()")

time.sleep(5)
send_command(arduino, "home(J3)")

time.sleep(5)
send_command(arduino, "flip(0)")

send_command(arduino, f"coords(J1,25,0)")
time.sleep(5)

send_command(arduino, f"coords(J2,25,10000)")
time.sleep(5)

send_command(arduino, "open()")
time.sleep(5)

send_command(arduino, "home(J1)")
time.sleep(5)

send_command(arduino, "home(J2)")
time.sleep(5)


time.sleep(5)

print("Cylinder delivered.")

send_command(arduino, "cameraPose()")

time.sleep(5)

send_command(arduino, "flip(180)")
time.sleep(5)

send_command(arduino, "close()")
time.sleep(5)

print("Returning to vision loop.")

arduino.close()