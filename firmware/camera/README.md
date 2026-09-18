# Camera firmware

Runs on the vision system's ESP32-S3 AI camera module (separate board from
the arm controller).

- **`CameraWebServer.ino`** — the version actually used on the robot. Serves
  a `/capture` HTTP endpoint over Wi-Fi so the host laptop
  ([`../../software/vision-pick-and-place/`](../../software/vision-pick-and-place))
  can pull a still frame on demand.
- **`image_processing_onboard_detector.ino`** — an alternative approach that
  was prototyped: doing bright-pixel centroid detection *on the camera board
  itself* instead of sending the frame to the host. The host-side OpenCV
  pipeline is what the final system used instead.

## Note

Set your own Wi-Fi SSID/password at the top of `CameraWebServer.ino` before
flashing — the camera and host laptop need to be on the same network.
