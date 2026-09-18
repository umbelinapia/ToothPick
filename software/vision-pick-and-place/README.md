# Vision pick-and-place controller

Runs on a host laptop, talking to two ESP32-S3 boards
([`../../firmware/`](../../firmware)):

- **Final:** [`final/pick_and_place_controller.py`](final/pick_and_place_controller.py) —
  pulls a frame from the camera over HTTP, runs OpenCV Hough Circle detection
  to find the disc, maps its pixel position to robot coordinates, and drives
  the arm ([`../../firmware/arm-controller/final/arm_controller.ino`](../../firmware/arm-controller/final/arm_controller.ino))
  over serial through the full pick/flip/drop cycle.

## Iteration history

`iterations/` (numbered in build order) is the path from "can we grab a frame
and see anything" to the final integrated controller:

- `01`–`03` — basic HTTP capture from the ESP32 camera
- `04`–`07` — first circle/blob detection experiments
- `08` — scratch/draft file
- `09`–`10` — vision detection wrapped in a repeatable loop
- `11`–`12` — vision loop wired up to serial commands to the arm
- `13` — full automated infinite-loop version, immediate predecessor of
  `final/pick_and_place_controller.py`

## Detection results

[`../../evidence/photos/hough-circles-detection-output.png`](../../evidence/photos/hough-circles-detection-output.png)
is a real output frame from the detection pipeline (circle found and
annotated). [`../../evidence/photos/vision-test-captures/`](../../evidence/photos/vision-test-captures)
holds the raw top-down workspace photos — with and without the target disc,
at different positions — used to develop and tune the Hough-circle parameters
(`dp`, `minDist`, `param1`/`param2`, `minRadius`/`maxRadius` in
[`final/pick_and_place_controller.py`](final/pick_and_place_controller.py)).

See [`../../evidence/documentation/architecture-workflow.md`](../../evidence/documentation/architecture-workflow.md)
for the full pipeline diagram (image acquisition → processing → detection →
coordinate transform) and the two-loop serial handshake between this script
and the arm firmware.
