// ============================================================
//  White Cylinder Detector — DFRobot ESP32-S3 AI Camera v1.1
//  
//  What this sketch does:
//    1. Starts the camera in grayscale mode at low resolution
//    2. Waits for a trigger (serial command or just runs once)
//    3. Captures a single frame
//    4. Scans every pixel and finds the bright (white) ones
//    5. Calculates the centroid (centre point) of all bright pixels
//    6. Also calculates a bounding box (min/max extents)
//    7. Prints the results over Serial as plain text and JSON
//
//  Wiring / board:
//    Board: "ESP32S3 Dev Module" (or DFRobot specific board if installed)
//    No extra wiring needed — camera is on-board
//
//  Serial output (115200 baud):
//    READY — printed once setup is complete
//    Each detection prints:
//      RAW:  cx=160 cy=120 pixels=4821 box=120,80,200,160
//      JSON: {"cx":160,"cy":120,"pixels":4821,"box_x1":120,"box_y1":80,"box_x2":200,"box_y2":160,"valid":true}
// ============================================================

#include "esp_camera.h"   // Espressif camera driver — handles capture + pixel formats

// ── Camera pin definitions for DFRobot ESP32-S3 AI Camera v1.1 ──────────────
// These are the GPIO numbers that connect the ESP32-S3 to the OV2640 sensor.
// If your board variant uses different pins, adjust these to match your schematic.
#define PWDN_GPIO_NUM   -1   // Power down pin — not used on this board (-1 = disabled)
#define RESET_GPIO_NUM  -1   // Hardware reset — not used
#define XCLK_GPIO_NUM    5   // Clock signal to camera sensor
#define SIOD_GPIO_NUM    8   // I2C data line (camera config bus)
#define SIOC_GPIO_NUM    9   // I2C clock line (camera config bus)
#define Y9_GPIO_NUM      4   // \
#define Y8_GPIO_NUM      6   //  |
#define Y7_GPIO_NUM      7   //  |  8-bit parallel pixel data bus
#define Y6_GPIO_NUM     14   //  |  Y2 = LSB, Y9 = MSB
#define Y5_GPIO_NUM     17   //  |
#define Y4_GPIO_NUM     21   //  |
#define Y3_GPIO_NUM     18   //  |
#define Y2_GPIO_NUM     16   // /
#define VSYNC_GPIO_NUM   1   // Vertical sync — marks start of new frame
#define HREF_GPIO_NUM    2   // Horizontal reference — marks active pixel rows
#define PCLK_GPIO_NUM   15   // Pixel clock — one pulse per pixel byte

// ── Detection settings ────────────────────────────────────────────────────────

// THRESHOLD: pixel brightness (0=black, 255=white) above which we consider
// a pixel to be part of the white cylinder.
// 200 works well for a bright white object on a black background under
// consistent lighting. Lower this (e.g. 150) if your white looks grey.
// Raise it (e.g. 230) if you get false positives from stray light.
#define BRIGHTNESS_THRESHOLD  200

// MINIMUM PIXELS: how many bright pixels must be found before we report
// a valid detection. This filters out single hot pixels or tiny reflections.
// For a cylinder that fills maybe 10-30% of the frame at QVGA (320x240 = 76800px),
// even 500 is very conservative. Tune upward if you get spurious detections.
#define MIN_PIXEL_COUNT  500

// ── Setup ─────────────────────────────────────────────────────────────────────

void setup() {
  Serial.begin(115200);
  delay(1000);  // Give serial monitor time to connect before we print anything
  Serial.println("\n\n=== White Cylinder Detector ===");

  // Configure the camera driver
  camera_config_t config;

  config.ledc_channel = LEDC_CHANNEL_0;  // LEDC peripheral used to generate XCLK
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0       = Y2_GPIO_NUM;     // Map data pins
  config.pin_d1       = Y3_GPIO_NUM;
  config.pin_d2       = Y4_GPIO_NUM;
  config.pin_d3       = Y5_GPIO_NUM;
  config.pin_d4       = Y6_GPIO_NUM;
  config.pin_d5       = Y7_GPIO_NUM;
  config.pin_d6       = Y8_GPIO_NUM;
  config.pin_d7       = Y9_GPIO_NUM;
  config.pin_xclk     = XCLK_GPIO_NUM;
  config.pin_pclk     = PCLK_GPIO_NUM;
  config.pin_vsync    = VSYNC_GPIO_NUM;
  config.pin_href     = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;   // SCCB = Serial Camera Control Bus (= I2C)
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn     = PWDN_GPIO_NUM;
  config.pin_reset    = RESET_GPIO_NUM;

  config.xclk_freq_hz = 20000000;        // 20 MHz clock to sensor — standard value

  // PIXFORMAT_GRAYSCALE: each pixel is a single byte (0–255 brightness).
  // This is the key choice — no colour information, half the data vs RGB,
  // and perfect for our use case (white on black doesn't need colour).
  config.pixel_format = PIXFORMAT_GRAYSCALE;

  // FRAMESIZE_QVGA = 320 × 240 pixels = 76,800 bytes per frame.
  // Plenty of resolution to locate a cylinder, and small enough to process
  // quickly in RAM. Go lower (FRAMESIZE_QQVGA = 160x120) if you need speed.
  config.frame_size   = FRAMESIZE_QVGA;

  config.jpeg_quality = 10;  // Not used for GRAYSCALE, but required field
  config.fb_count     = 1;   // One frame buffer — we capture one shot at a time
  config.fb_location  = CAMERA_FB_IN_PSRAM;  // Store frame in PSRAM (8MB on this board)
                                              // Without PSRAM, QVGA grayscale still fits
                                              // in internal RAM, but PSRAM is safer

  // Initialise the camera hardware
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    // If this fails, check your pin definitions match the actual board schematic
    Serial.printf("Camera init FAILED: 0x%x\n", err);
    Serial.println("Check pin definitions and board selection.");
    while (true) { delay(1000); }  // Halt — nothing we can do without a camera
  }

  Serial.println("Camera init OK");
  Serial.printf("Resolution: 320 x 240 (QVGA), Grayscale\n");
  Serial.printf("Brightness threshold: %d / 255\n", BRIGHTNESS_THRESHOLD);
  Serial.printf("Min pixel count for valid detection: %d\n\n", MIN_PIXEL_COUNT);
  Serial.println("READY");
  Serial.println("Send any character over Serial to trigger a capture, or it auto-captures every 3 seconds.");
  Serial.println("─────────────────────────────────────────────────");
}

// ── Main loop ─────────────────────────────────────────────────────────────────

void loop() {
  // Option A — trigger on serial input (type anything in Serial Monitor)
  // Option B — auto-capture every 3 seconds (comment out the if/wait below)
  
  // Wait for serial input OR 3-second timeout
  unsigned long start = millis();
  bool triggered = false;
  while (millis() - start < 3000) {
    if (Serial.available()) {
      while (Serial.available()) Serial.read();  // flush input buffer
      triggered = true;
      break;
    }
  }
  // triggered == true  → user sent a character
  // triggered == false → 3-second timer elapsed (auto mode)

  Serial.println("\nCapturing frame...");
  captureAndDetect();
}

// ── Core detection function ───────────────────────────────────────────────────

void captureAndDetect() {

  // ── 1. Grab a frame from the camera ──────────────────────────────────────
  // esp_camera_fb_get() asks the driver to capture one frame into a buffer.
  // It blocks until the frame is ready (a few milliseconds).
  camera_fb_t *fb = esp_camera_fb_get();

  if (!fb) {
    Serial.println("ERROR: Failed to capture frame. Camera may be busy.");
    return;
  }

  // fb->buf    — pointer to raw pixel data
  // fb->len    — total number of bytes (should be width * height for grayscale)
  // fb->width  — frame width in pixels (320 for QVGA)
  // fb->height — frame height in pixels (240 for QVGA)
  // fb->format — should be PIXFORMAT_GRAYSCALE

  int width  = fb->width;   // 320
  int height = fb->height;  // 240
  uint8_t *pixels = fb->buf; // Array of brightness values, row by row, left to right

  Serial.printf("Frame captured: %d x %d = %d bytes\n", width, height, fb->len);

  // ── 2. Threshold scan + centroid accumulation ─────────────────────────────
  //
  // We scan every pixel exactly once.
  // For each pixel brighter than BRIGHTNESS_THRESHOLD:
  //   - add its X position to sum_x
  //   - add its Y position to sum_y
  //   - increment count
  //   - update bounding box (min/max x and y)
  //
  // After the scan:
  //   centroid_x = sum_x / count
  //   centroid_y = sum_y / count
  //
  // This is the "centre of mass" of all bright pixels — the geometric
  // centre of the white cylinder top as seen by the camera.

  long sum_x   = 0;    // Sum of X coordinates of all bright pixels
  long sum_y   = 0;    // Sum of Y coordinates
  long count   = 0;    // How many bright pixels we found

  // Bounding box: we track the extremes so we know the cylinder's extent
  int box_x1 = width;   // leftmost bright pixel  (start at max, shrink inward)
  int box_y1 = height;  // topmost bright pixel
  int box_x2 = 0;       // rightmost bright pixel (start at min, grow outward)
  int box_y2 = 0;       // bottommost bright pixel

  // Outer loop = rows (Y axis, top to bottom)
  for (int y = 0; y < height; y++) {
    // Inner loop = columns (X axis, left to right)
    for (int x = 0; x < width; x++) {

      // pixels[] is a flat 1D array. To get pixel at column x, row y:
      //   index = y * width + x
      // This is standard row-major (C-style) 2D array indexing.
      uint8_t brightness = pixels[y * width + x];

      if (brightness > BRIGHTNESS_THRESHOLD) {
        // This pixel is bright enough to be part of the white cylinder
        sum_x += x;
        sum_y += y;
        count++;

        // Update bounding box
        if (x < box_x1) box_x1 = x;
        if (x > box_x2) box_x2 = x;
        if (y < box_y1) box_y1 = y;
        if (y > box_y2) box_y2 = y;
      }
    }
  }

  // ── 3. Calculate results ──────────────────────────────────────────────────

  bool valid = (count >= MIN_PIXEL_COUNT);
  // "valid" means we found enough bright pixels to be confident there's
  // actually a cylinder there, not just noise or a stray reflection.

  int cx = 0, cy = 0;          // Centroid coordinates
  int blob_w = 0, blob_h = 0;  // Bounding box width and height

  if (valid) {
    // Integer division is fine here — sub-pixel accuracy isn't needed
    cx = (int)(sum_x / count);
    cy = (int)(sum_y / count);
    blob_w = box_x2 - box_x1;
    blob_h = box_y2 - box_y1;
  }

  // ── 4. Coordinates relative to camera centre ──────────────────────────────
  //
  // Raw pixel coordinates (cx, cy) are measured from the top-left corner.
  // It's more useful to express position relative to the image centre,
  // so that:
  //   rel_x = 0, rel_y = 0  means the cylinder is dead centre in frame
  //   rel_x > 0             means it's to the right of centre
  //   rel_y > 0             means it's below centre
  //
  // This makes it easy to drive motors or report offset direction.

  int centre_x = width  / 2;  // 160 for QVGA
  int centre_y = height / 2;  // 120 for QVGA

  int rel_x = cx - centre_x;  // Positive = right of centre
  int rel_y = cy - centre_y;  // Positive = below centre (Y increases downward)
                               // Negate rel_y if you prefer +Y = up

  // ── 5. Print results ──────────────────────────────────────────────────────

  if (valid) {
    // RAW line: human-readable summary
    Serial.printf("RAW:  cx=%d cy=%d | rel=(%+d, %+d) | pixels=%ld | box=%d,%d,%d,%d (w=%d h=%d)\n",
      cx, cy, rel_x, rel_y, count, box_x1, box_y1, box_x2, box_y2, blob_w, blob_h);

    // JSON line: machine-readable, easy to parse by a host computer if needed
    Serial.printf("JSON: {\"cx\":%d,\"cy\":%d,\"rel_x\":%d,\"rel_y\":%d,"
                  "\"pixels\":%ld,\"box_x1\":%d,\"box_y1\":%d,"
                  "\"box_x2\":%d,\"box_y2\":%d,\"valid\":true}\n",
      cx, cy, rel_x, rel_y, count, box_x1, box_y1, box_x2, box_y2);

    // Optional: simple text description of position
    Serial.print("POS:  Cylinder is ");
    if      (rel_x < -width/6)  Serial.print("LEFT");
    else if (rel_x >  width/6)  Serial.print("RIGHT");
    else                         Serial.print("CENTRE-X");
    Serial.print(" / ");
    if      (rel_y < -height/6) Serial.print("TOP");
    else if (rel_y >  height/6) Serial.print("BOTTOM");
    else                         Serial.print("CENTRE-Y");
    Serial.println();

  } else {
    // Not enough bright pixels — either no cylinder, or threshold needs tuning
    Serial.printf("NO DETECTION — only %ld bright pixels found (need >= %d)\n",
      count, MIN_PIXEL_COUNT);
    Serial.printf("JSON: {\"valid\":false,\"pixels\":%ld}\n", count);
    Serial.println("Tip: Lower BRIGHTNESS_THRESHOLD if the cylinder looks dim.");
  }

  // ── 6. Release the frame buffer ──────────────────────────────────────────
  // IMPORTANT: always call this after you're done with fb.
  // If you skip this, the camera driver runs out of buffers and stops working.
  esp_camera_fb_return(fb);

  Serial.println("─────────────────────────────────────────────────");
}
