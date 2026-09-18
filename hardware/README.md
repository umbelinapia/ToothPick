# Hardware

3-DOF Cartesian gantry robot:

- Gearbox drivetrain (IGUS Apiro modular drive units) on toothed-belt rails
- 3 stepper motors (joints J1/J2/J3) + 2 micro servos (gripper open/close, flip)
- Homing limit switches on every joint for a repeatable reference position
- **ESP32-S3-DEV2** as the main arm controller (compact, WiFi/Bluetooth, drives
  steppers + servos, exposes a serial command API — see [`../firmware/`](../firmware))
- Separate **ESP32-S3 AI camera module** (OV3660) for vision, wired to a
  stripboard to keep wiring permanent and minimise disconnects
- 3D-printed structural and end-effector parts

## In this folder

- [`cad/`](cad) — STL exports of the team's own designed/printed parts,
  grouped by subsystem. See [`cad/README.md`](cad/README.md).
- [`datasheets/`](datasheets) — component datasheets: MG90 servo dimensions,
  OV3660 camera sensor datasheet/specs, its onboard microphone, and the
  ESP32-S3 AI camera module itself.
