# Toothpick — Autonomous Cylinder Pick System

3rd-year Mechatronics group project (Loughborough University, Group 16).

**"SEE IT. PICK IT. SORT IT."** A vision-guided 3-DOF gantry robot that locates a
cylindrical disc with an onboard camera, picks it up, flips the end-effector, and
places it at a drop location — fully autonomously, with no manual coordinate entry.

## Team

Group 16: **Umbelina**, **Nikitha Prabhakar**, **Max**, **Daniel**.

## What it does

1. The arm homes all three joints against limit switches, then moves to a fixed
   camera pose.
2. An ESP32-S3 AI camera (OV3660 sensor) captures a frame of the workspace.
3. A host-side Python controller pulls the image over Wi-Fi, runs a Hough Circle
   detection pass to find the cylindrical disc, and maps its pixel coordinates
   to robot joint coordinates (with a special "edge routine" for parts detected
   near the boundary of the workspace).
4. The controller drives the arm over serial to pick the disc, home, flip the
   end-effector, move to the drop position, and release — then returns home.

Target part (measured, see [`evidence/measurements/`](evidence/measurements)):
a disc of mean diameter **25.76 mm** and mean height **6.26 mm**.

## Hardware

- 3-DOF gantry-style robot arm: gearbox drivetrain on toothed rails
- 3 stepper motors (joints J1/J2/J3) + 2 micro servos (gripper open/close, flip)
- Homing limit switches on every joint for a repeatable reference position
- **ESP32-S3-DEV2** as the main arm controller (compact, WiFi/Bluetooth, drives
  steppers + servos, exposes a serial command API)
- Separate **ESP32-S3 AI camera module** (OV3660) for vision, wired to a
  stripboard to minimise loose wiring and disconnects
- 3D-printed structural/end-effector parts — CAD in [`hardware/cad/`](hardware/cad)

## System architecture

| Layer | Location | Role |
|---|---|---|
| Arm firmware | [`firmware/arm-controller/final/FINALONE.ino`](firmware/arm-controller/final/FINALONE.ino) | Runs on the arm's ESP32-S3. Drives steppers/servos, handles homing, and exposes a serial command API (`home(J1)`, `coords(J2,100,8000)`, `open()`, `close()`, `flipUp()`, `flipDown()`, `cameraPose()`, `pos()`). Enforces safety interlocks — flipping is only allowed when J3 is at true home, and drop-open is only allowed when J1 is at home. |
| Camera firmware | [`firmware/camera/CameraWebServer.ino`](firmware/camera/CameraWebServer.ino) | Runs on the camera's ESP32-S3. Serves a `/capture` HTTP endpoint so the host can pull a still frame over Wi-Fi. |
| Host controller | [`software/vision-pick-and-place/final/FINALCODEMAX.py`](software/vision-pick-and-place/final/FINALCODEMAX.py) | Runs on a laptop. Talks to the arm over serial and the camera over HTTP, does the OpenCV disc detection + coordinate mapping, and drives the full pick → flip → drop cycle described above. |

Each code layer has a matching `iterations` folder showing how it got there —
see [`firmware/arm-controller/README.md`](firmware/arm-controller/README.md)
and [`software/vision-pick-and-place/README.md`](software/vision-pick-and-place/README.md).
Full system diagrams (sequencing, vision pipeline, why a gantry robot) are in
[`evidence/documentation/architecture.md`](evidence/documentation/architecture.md),
transcribed from the project's own "robot control logic" deck.

## Repository layout

```
firmware/     Code that runs on the two ESP32-S3 boards (arm controller + camera)
software/     Host-side (laptop) vision + pick-and-place controller
hardware/     CAD (STL exports) and component datasheets
evidence/     Documentation, presentations, measurements, build/test photos and demo video
```

Each top-level folder has its own README (start there): [`firmware/README.md`](firmware/README.md),
[`software/README.md`](software/README.md), [`hardware/README.md`](hardware/README.md),
[`evidence/README.md`](evidence/README.md).

## Requirements, tasks & outcomes

See [`evidence/documentation/tasks-and-requirements.md`](evidence/documentation/tasks-and-requirements.md)
for the task breakdown by subsystem and owner,
[`evidence/documentation/architecture.md`](evidence/documentation/architecture.md)
for how the system works end to end, and
[`evidence/documentation/presentations/`](evidence/documentation/presentations)
for the full project pitch and viva decks.

## Demo & build photos

- [`evidence/videos/pick-and-place-demo.MOV`](evidence/videos/pick-and-place-demo.MOV) —
  the arm running a full pick → flip → place cycle
- [`evidence/photos/build-and-dev/`](evidence/photos/build-and-dev) — the physical
  build (gantry, wiring, stripboard) and the dev setup mid-debug (live camera
  stream + the Hough-circle detection script running side by side)
- [`evidence/photos/vision-test-captures/`](evidence/photos/vision-test-captures) — raw
  workspace camera captures used to develop and tune the disc-detection routine
- [`evidence/photos/hough-circles-detection-output.png`](evidence/photos/hough-circles-detection-output.png) —
  an annotated detection result from the pipeline

## Notes on what's included

- CAD here is limited to **STL exports of the team's own parts**, grouped by
  subsystem. The native SolidWorks/Inventor source files are large, proprietary
  formats and are kept with the original project files rather than duplicated
  here.
- Third-party reference geometry (vendor robot-arm STEP files used only for
  dimension comparisons during design) and an unrelated pitch deck from a
  different module that had been saved in the same working folder were left
  out — they aren't part of this project's own work.
- A hardcoded Wi-Fi SSID/password in the camera firmware was replaced with
  placeholders before publishing; a near-duplicate test sketch that contained
  a real personal Wi-Fi password was excluded entirely rather than kept.
