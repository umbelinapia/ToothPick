# 3rd year Mechatronics group project

Loughborough University, Module 25WSC912 (Mechatronics and Instrumentation),
CW2: *Design and Build of a Disc Flipping and Positioning Mechanism*.
Group 16 — **Umbelina**, **Nikitha Prabhakar**, **Max**, **Daniel**.
Project codename: **Toothpick**.

## Project aim

A manufacturer's forming machine outputs plastic disc components in the
wrong orientation and an undefined location. This used to be fixed by a
worker by hand — with a 1% misorientation rate, and no longer safe as the
line moves to remote, unmanned automation. Our aim was to design, build, and
test an automated system that locates, flips, and places the disc reliably,
with no person in the loop.

## Project tasks

- Locate the disc using sensors (position is uncertain, so vision sensing is
  used even when not strictly required)
- Flip the disc upside down ("heads" to "tails")
- Pick up the disc and place it at a fixed destination
- Run repeatable experiments picking the disc from random starting positions

## Project objectives

- 3-DOF gantry robot built from IGUS Apiro modular drive gear units,
  controlled by an ESP32-S3
- Vision-guided disc detection (camera + OpenCV), not fixed coordinates
- Reliable, repeatable homing so every cycle starts from a known reference
- Safety interlocks so the flip/drop motions can't happen in an unsafe joint
  position
- Fast, consistent cycle time — assessed live as 4 runs in a 6-minute demo
- Stay within the £50 build budget

## Project outcomes

- Working end-to-end pipeline: home → move to camera pose → detect disc →
  pick → home → flip → move to drop → place → home
  (see [`firmware/`](firmware) and [`software/`](software))
- Vision pipeline validated against the measured part: mean diameter
  **25.76 mm**, mean height **6.26 mm** — see [`evidence/measurements/`](evidence/measurements)
- Full sequence built, tested, and demonstrated at the Viva Voce — see
  [`evidence/documentation/presentations/`](evidence/documentation/presentations)
  and [`evidence/videos/pick-and-place-demo.MOV`](evidence/videos/pick-and-place-demo.MOV)

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
[`evidence/README.md`](evidence/README.md). Task breakdown by subsystem and
owner is in [`evidence/documentation/tasks-and-requirements.md`](evidence/documentation/tasks-and-requirements.md).

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
