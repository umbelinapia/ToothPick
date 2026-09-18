# Vision system

Two microcontrollers plus a host laptop:

- **Camera ESP32-S3** — runs [`esp32-camera-firmware/CameraWebServer.ino`](esp32-camera-firmware/CameraWebServer.ino),
  serving a `/capture` HTTP endpoint over Wi-Fi. `esp32-camera-firmware/image_processing_onboard_detector.ino`
  is an alternative approach that was prototyped — doing the bright-pixel
  centroid detection *on the camera board itself* instead of on the host —
  but the host-side OpenCV pipeline below is what the final system used.
- **Host laptop** — runs [`final/FINALCODEMAX.py`](final/FINALCODEMAX.py), which
  pulls a frame from the camera, runs OpenCV Hough Circle detection to find the
  disc, maps its pixel position to robot coordinates, and drives the arm
  (`firmware/`) over serial through the full pick/flip/drop cycle.

## Iteration history

`python-iterations/` (numbered in build order) is the path from "can we grab
a frame and see anything" to the final integrated controller:

- `01`–`03` — basic HTTP capture from the ESP32 camera
- `04`–`07` — first circle/blob detection experiments
- `08` — scratch/draft file
- `09`–`10` — vision detection wrapped in a repeatable loop
- `11`–`12` — vision loop wired up to serial commands to the arm
- `13` — full automated infinite-loop version, immediate predecessor of
  `final/FINALCODEMAX.py`

## Camera hardware

Datasheets for the sensor and module are in [`datasheets/`](datasheets):
OV3660 image sensor, its onboard microphone, and the ESP32-S3 AI camera module
itself. Build photos are in [`images/`](images).

## Note

The Wi-Fi SSID/password in `CameraWebServer.ino` were placeholders here —
the original hardcoded credentials (including a real personal hotspot
password found in one duplicate test sketch) were removed before publishing.
