# Requirements, tasks & outcomes

Derived from [`gantt-chart.xlsx`](gantt-chart.xlsx) and the project decks in
[`presentations/`](presentations).

## Product goal

**Autonomous Cylinder Pick System** — reliably detect, pick, flip and place a
cylindrical disc without manual coordinate entry. Design drivers: accurate,
repeatable, simple; a stable structure; suited to the target environment.

## Requirements identified

- 3-DOF gantry robot with a gearbox drivetrain on toothed rails
- 3 stepper motors + 2 micro servo motors (gripper, flip)
- Homing system (limit switches) for a repeatable reference position on every cycle
- ESP32-S3-DEV2 controller: compact, sufficient processing headroom, WiFi/Bluetooth
- Stripboard wiring to keep wiring permanent and minimise disconnects
- Vision system able to locate the target disc and hand off coordinates to the arm
- Modular, adaptable, intuitive firmware functions (scalable to new end-effector routines)

Target part, measured (see [`../results/disk-measurement-data.xlsx`](../results/disk-measurement-data.xlsx)):
mean diameter **25.76 mm**, mean height **6.26 mm** (5 samples).

## Sequencing rules (safety/consistency)

- Flipping the end-effector is only permitted when J3 is at its true home position
- Dropping (opening the gripper at the drop point) is only permitted when J1 is at home
- Every serial command blocks until the arm reports `DONE`, so each movement
  completes before the next is issued
- The homing routine re-establishes a repeatable reference position at the
  start and end of every cycle

## Task breakdown by subsystem and owner

**Preliminary** (all, led by Umbelina) — success criteria definition, workspace
dimensions, disc specifications, requirements identification, task
repartition, Gantt chart, camera selection, Bill of Materials.

**Mechanical**
- Body (Umbelina): layout vs. workspace, arm architecture selection, link
  length calculations, reachability analysis
- End-effector (Max): flip mechanism concept, gripper type selection, gripper
  + mounting CAD design
- Fabrication (Max/Umbelina): CAD → print, parts ordering, assembly

**Electrical**
- Power (Nikitha): power requirement calculation, power supply selection
- Control hardware (Nikitha): ESP32 configuration, end-effector actuator
  driver, sensor selection
- Integration (Max/Umbelina): wiring diagram, wiring, cable management, safety
  switch integration

**Software**
- Low-level control (Nikitha): ESP32 environment setup, motor driver
  interface code, encoder reading
- Motion (Umbelina): joint limits, motion profiling/speed, pick/flip/place
  sequence routines
- Vision (Umbelina): camera calibration, pixel→mm workspace calibration, disc
  detection algorithm, coordinate transform to robot frame

**Testing** — single-joint test, multi-joint coordinated motion test, dry run
without the disc, pick test, flip test, full pipeline test.

**Validation** (Daniel) — accuracy measurement, flip success rate measurement,
cycle time measurement.

## Outcomes

See [`presentations/`](presentations) (Group 16 Viva deck, Mechatronics
Presentation deck) for the final system description and demonstrated
sequence: home → move to camera pose → detect disc → pick → home →
flip → move to drop → lower → drop → home, implemented end-to-end in
[`../vision-system/final/FINALCODEMAX.py`](../vision-system/final/FINALCODEMAX.py)
driving [`../firmware/final/FINALONE.ino`](../firmware/final/FINALONE.ino).
