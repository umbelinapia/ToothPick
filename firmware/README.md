# Firmware

Code that runs directly on the two ESP32-S3 microcontrollers.

- [`arm-controller/`](arm-controller): drives the gantry robot, steppers,
  servos, homing, and the serial command API. See
  [`arm-controller/README.md`](arm-controller/README.md) for the final version
  vs. build-order iteration history.
- [`camera/`](camera): runs on the vision system's ESP32-S3 AI camera module.
  See [`camera/README.md`](camera/README.md).
