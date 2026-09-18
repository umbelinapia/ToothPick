# Architecture

Full source diagrams: [`robot-control-logic.pdf`](robot-control-logic.pdf) (this
is also where the repo name comes from — "toothpick" was the project's internal
codename, after the toothed-rail gantry drivetrain).

## System overview

```
        PYTHON (host laptop)                          ESP32-S3 (arm)
        ─────────────────────                          ───────────────
        image acquisition  ──HTTP GET /capture──▶  ESP32-S3 AI camera
        image processing (Gaussian blur)
        object detection (Hough circles)
        pixel → robot-frame transformation
              │
              ▼
        serial command  ───────UART────────────▶  arm control loop
        (cameraPose(), home(), open(), close(),      │
         flipUp()/flipDown(), coords(J,speed,pos))    ▼
                                                  task execution:
                                                  homing, stepper/servo motion,
                                                  safety interlocks
              ◀──────────"DONE" / "ERROR: ..."────────┘
```

## Robot structure

**Gantry robot, Cartesian configuration** — chosen over an articulated arm for
simple kinematics (no inverse-kinematics solve needed — each axis maps
directly to a Cartesian/joint coordinate), straightforward screw-and-nut
assembly, and simpler cable management along the linear rails.

## Vision pipeline

ESP32-S3 AI camera (wide-angle) → online MJPEG/HTTP stream for setup and
debugging → single-frame capture on trigger → Gaussian blur → Hough circle
detection → pixel coordinates of the detected disc → transform to robot-frame
coordinates. See [`hough-circles-detection-output.png`](../vision-system/images/hough-circles-detection-output.png)
for a real detection result, and [`test-captures/`](../vision-system/test-captures)
for the raw camera views used to develop and validate the detection routine.

## Task sequencing (two independent event loops, joined by serial + shared Wi-Fi)

**Python `loop()` (host):**
capture image → process image → cylinder detected? → no: return a warning and
retry → yes: get coordinates → execute task (send the serial command sequence
for pick/flip/place).

**Arduino `loop()` (arm):**
read serial input → is it one valid, recognised command? → no: return an
error over serial → yes: execute the command and report `DONE`.

This request/acknowledge handshake (host blocks on `DONE`/`ERROR` per command,
see `wait_for_done()` in [`FINALCODEMAX.py`](../vision-system/final/FINALCODEMAX.py))
is what keeps the two independently-running loops in lock-step despite
running on separate microcontrollers connected only by serial + a shared Wi-Fi
network for the camera link.

## Safety interlocks

- `flipUp()` / `flipDown()` only execute when J3 is at its true home switch
- `dropOpen()` only executes when J1 is at its home switch
- Both are enforced in firmware (`flipIsSafe()`, `dropIsSafe()` in
  [`FINALONE.ino`](../firmware/final/FINALONE.ino)), not just in the host
  script, so a bug in the Python sequencing can't command an unsafe motion
