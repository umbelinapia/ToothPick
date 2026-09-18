# Arm firmware

**Final:** [`final/arm_controller.ino`](final/arm_controller.ino) — the
version that ran on the robot. Controls 3 stepper joints (J1/J2/J3) and 2
servos (gripper, flip) on an ESP32-S3, with per-joint homing, a serial
command API, and safety interlocks (flip only at true J3 home, drop only at
J1 home).

**Iterations** (`iterations/`, in build order) — kept for the project record:

| Folder | What it was |
|---|---|
| `01_motor_test` | First single-motor step/pulse test |
| `02_motor_test_success` | Confirmed-working version of the same test |
| `03_umbelina_version` | First full arm sketch — joint definitions, calibration notes |
| `04_calibration` | Calibration pass on joint travel limits |
| `05_camera_serial` | Added the camera-pose move + serial handshake |
| `06_speed_test` | Experiment tightening step-pulse timing for faster motion |
| `07_umbelina_version_2` | Expanded control logic, more commands |
| `08_this_worksish` | First version with the full pick/flip/drop command set working |
| `09_umbelina_version_3` | Refined version, closest predecessor to final |
| `10_serial_heartbeat_stub` | Minimal serial "Ready/Looping" test stub, used to debug the serial link in isolation |
