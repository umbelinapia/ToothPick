import requests
from datetime import datetime
from pathlib import Path
import serial
import time

# SERIAL_PORT = "COM6"
# BAUDRATE = 115200

# import serial

# ser = serial.Serial("COM13", 115200)
# print("Opened successfully")
# # ser.close()

ESP32_URL = "http://172.20.10.2/capture"

# SAVE_DIR = Path("snapshots")
# SAVE_DIR.mkdir(exist_ok=True)
# # SERIAL_PORT = "COM13"
# # BAUDRATE = 115200

# def get_url():
#     with serial.Serial(SERIAL_PORT, BAUDRATE, timeout=1) as ser:
#         time.sleep(2)

#         while True:
#             line = ser.readline().decode(errors="ignore").strip()
#             if line.startswith("http://"):
#                 return line

# url = get_url()
# print("URL:", url)

# # take snapshot
# capture_url = url + "/capture"

# print("Using:", capture_url)

response = requests.get(ESP32_URL)
with open("board.jpg", "wb") as f:
    f.write(response.content)

print("Saved photo.jpg")


# def request_capture_url():
    # with serial.Serial(SERIAL_PORT, BAUDRATE, timeout=1) as ser:
        # time.sleep(2)
        # ser.write(b"IP\n")

        # deadline = time.time() + 5
        # while time.time() < deadline:
            # line = ser.readline().decode(errors="ignore").strip()
            # if line:
                # print("SERIAL:", line)

            # m = re.match(r"CAPTURE_URL=(http://\d+\.\d+\.\d+\.\d+/capture)", line)
            # if m:
                # return m.group(1)

    # raise TimeoutError("ESP32 did not return capture URL")

# print(request_capture_url())

# def take_snapshot():
    # timestamp = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
    # filename = SAVE_DIR / f"snapshot_{timestamp}.jpg"

    # response = requests.get(ESP32_URL, timeout=10)
    # response.raise_for_status()

    # content_type = response.headers.get("Content-Type", "")
    # if "image/jpeg" not in content_type:
        # raise ValueError(f"Unexpected content type: {content_type}")

    # with open(filename, "wb") as f:
        # f.write(response.content)

    # print(f"Saved: {filename}")
    # return filename

# if __name__ == "__main__":
    # take_snapshot()