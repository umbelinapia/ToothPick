# Requirements

**Product goal:** reliably detect, pick, flip and place a cylindrical disc
without manual coordinate entry. Design drivers: accurate, repeatable,
simple; a stable structure; suited to the target environment.

## Identified requirements

- 3-DOF gantry robot with a gearbox drivetrain on toothed rails
- 3 stepper motors + 2 micro servo motors (gripper, flip)
- Homing system (limit switches) for a repeatable reference position on every cycle
- ESP32-S3-DEV2 controller: compact, sufficient processing headroom, WiFi/Bluetooth
- Stripboard wiring to keep wiring permanent and minimise disconnects
- Vision system able to locate the target disc and hand off coordinates to the arm
- Modular, adaptable, intuitive firmware functions (scalable to new end-effector routines)
- Stay within the £50 build budget

Target part, measured (see [`../measurements/disk-measurement-data.xlsx`](../measurements/disk-measurement-data.xlsx)):
mean diameter **25.76 mm**, mean height **6.26 mm** (5 samples).

## Sequencing rules (safety/consistency)

- Flipping the end-effector is only permitted when J3 is at its true home position
- Dropping (opening the gripper at the drop point) is only permitted when J1 is at home
- Every serial command blocks until the arm reports `DONE`, so each movement
  completes before the next is issued
- The homing routine re-establishes a repeatable reference position at the
  start and end of every cycle
