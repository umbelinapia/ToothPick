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

Built and demonstrated a working end-to-end pipeline: home all joints, move
to a fixed camera pose, detect the disc, pick it up, home again, flip it,
move to the drop position, and place it — then reset for the next cycle. The
vision pipeline was validated against the actual part geometry (mean
diameter 25.76 mm, mean height 6.26 mm), and the full sequence was built,
tested, and demonstrated live at the Viva Voce.

## How the system works

The robot is a 3-DOF Cartesian gantry rather than an articulated arm, built
from IGUS Apiro modular drive gear units on toothed-belt rails. A gantry was
chosen over a rotary-jointed arm because each axis maps directly onto a
Cartesian/joint coordinate — no inverse-kinematics solve is needed to convert
a target position into motor commands, which kept both the mechanical build
and the control code simpler. Three stepper motors drive the three linear
joints (J1, J2, J3), and two micro servos handle the end-effector: one opens
and closes the gripper, the other flips it between "pick" and "drop"
orientation.

**Arm control.** An ESP32-S3 runs the low-level motion firmware. Each joint
is homed against a physical limit switch before every cycle, so the robot
always starts from a known, repeatable reference position rather than
trusting wherever it happened to stop last time. Each joint also has its own
step-pulse timing, tuned individually so lighter joints move quickly without
the heavier ones losing steps. On top of that sits a small text-based
command protocol read over serial — things like `home(J1)`, `coords(J2,
speed, position)`, `open()`, `close()`, `flipUp()`, `flipDown()`, and
`cameraPose()` — where every command blocks until the arm reports back
`DONE` or `ERROR: ...`, so the caller always knows exactly when a move has
actually finished. Two safety interlocks are enforced in the firmware itself,
not just in the higher-level script: the gripper can only flip when J3 is at
its true home position, and it can only drop at the destination when J1 is
at home — both physically the only positions where those motions are safe.

**Vision.** A second ESP32-S3 with a camera module serves single frames over
Wi-Fi on request. The host laptop pulls a frame, converts it to grayscale,
applies a Gaussian blur to cut down noise, and runs a Hough Circle Transform
tuned to the disc's expected radius. Exactly one circle is expected per
frame — zero or more than one is treated as a detection failure rather than
guessed at. The detected pixel coordinate is then mapped to robot-frame
coordinates using a linear transform calibrated against the physical
workspace, with an extra correction term for the fact that the camera looks
down at an angle rather than straight overhead, which otherwise skews the
mapping near the far edge of the workspace. If the mapped position falls too
close to the front edge of the workspace, a separate "edge routine" kicks
in — the gripper approaches from the side at that boundary instead of
straight down, since a normal vertical approach would risk pushing the disc
off the edge.

**Putting it together.** The host script and the arm firmware run as two
independent loops connected only by serial (plus Wi-Fi for the camera
frames). The host's loop captures an image, processes it, and — once it has
a valid detection — walks through the pick sequence one command at a time:
close the gripper, home all joints, flip up, move to the camera pose,
capture and detect, move over the disc (or run the edge routine), open,
lower, grab, return to J3 home, flip down, move to the drop position while
staying raised, lower, release, raise again, and home everything ready for
the next cycle. The arm's own loop just reads one command at a time off
serial, checks it's valid, executes it, and reports back — it has no idea
what the overall task is, it only ever sees one instruction at a time. The
blocking `DONE`/`ERROR` handshake between the two is what keeps them in
lock-step despite running on separate boards.

## Repository layout

```
firmware/     Code that runs on the two ESP32-S3 boards (arm controller + camera)
software/     Host-side (laptop) vision + pick-and-place controller
hardware/     CAD (STL exports) and component datasheets
evidence/     Documentation, presentations, measurements, build/test photos and demo video
```

Each folder has its own README with the details for that part of the
project, including the build-order history behind the final code and CAD.
