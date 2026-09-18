#include "esp_camera.h"
#include <WiFi.h>

// ===========================
// Select camera model in board_config.h
// ===========================
#include "board_config.h"

// ===========================
// Enter your WiFi credentials
// (redacted for public repo — set your own hotspot/router SSID + password)
// ===========================
const char *ssid = "YOUR_WIFI_SSID";
const char *password = "YOUR_WIFI_PASSWORD";

void startCameraServer();
void setupLedFlash();

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_UXGA;
  config.pixel_format = PIXFORMAT_JPEG;  // for streaming
  //config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if (config.pixel_format == PIXFORMAT_JPEG) {
    if (psramFound()) {
      config.jpeg_quality = 10;
      config.fb_count = 2;
      config.grab_mode = CAMERA_GRAB_LATEST;
    } else {
      // Limit the frame size when PSRAM is not available
      config.frame_size = FRAMESIZE_SVGA;
      config.fb_location = CAMERA_FB_IN_DRAM;
    }
  } else {
    // Best option for face detection/recognition
    config.frame_size = FRAMESIZE_240X240;
#if CONFIG_IDF_TARGET_ESP32S3
    config.fb_count = 2;
#endif
  }

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t *s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);        // flip it back
    s->set_brightness(s, 1);   // up the brightness just a bit
    s->set_saturation(s, -2);  // lower the saturation
  }
  // drop down frame size for higher initial frame rate
  if (config.pixel_format == PIXFORMAT_JPEG) {
    s->set_framesize(s, FRAMESIZE_QVGA);
  }

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

#if defined(CAMERA_MODEL_ESP32S3_EYE)
  s->set_vflip(s, 1);
#endif

// Setup LED FLash if LED pin is defined in camera_pins.h
#if defined(LED_GPIO_NUM)
  setupLedFlash();
#endif
  
  int n = WiFi.scanNetworks();

  if (n == 0) {
    Serial.println("No networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found:\n");

    for (int i = 0; i < n; i++) {
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" | RSSI: ");
      Serial.print(WiFi.RSSI(i));
      Serial.print(" dBm | Channel: ");
      Serial.print(WiFi.channel(i));
      Serial.print(" | Encryption: ");

      wifi_auth_mode_t enc = WiFi.encryptionType(i);

      switch (enc) {
        case WIFI_AUTH_OPEN: Serial.println("OPEN"); break;
        case WIFI_AUTH_WEP: Serial.println("WEP"); break;
        case WIFI_AUTH_WPA_PSK: Serial.println("WPA"); break;
        case WIFI_AUTH_WPA2_PSK: Serial.println("WPA2"); break;
        case WIFI_AUTH_WPA_WPA2_PSK: Serial.println("WPA/WPA2"); break;
        case WIFI_AUTH_WPA2_ENTERPRISE: Serial.println("WPA2-ENTERPRISE"); break;
        case WIFI_AUTH_WPA3_PSK: Serial.println("WPA3"); break;
        case WIFI_AUTH_WPA2_WPA3_PSK: Serial.println("WPA2/WPA3"); break;
        default: Serial.println("UNKNOWN"); break;
      }

      delay(10);
    }
  }

  // WiFi.mode(WIFI_STA);
  // WiFi.disconnect(true, true);
  // delay(1000);

  // // WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  // WiFi.setTxPower(WIFI_POWER_8_5dBm);
  // WiFi.setSleep(false);

  WiFi.begin(ssid, password);
  WiFi.setSleep(false);

  // Scan for networks to get the BSSID with the strongest signal


  //////////original
  Serial.print("WiFi connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  startCameraServer();

  // Serial.print("Camera Ready! Use 'http://");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  // Serial.println("' to connect");

  // /// grab the frame directly from the camera buffer ///
  // camera_fb_t *fb = esp_camera_fb_get();

  // if (!fb) {
  //   Serial.println("Camera capture failed");
  // } else {
  //   Serial.printf("Captured! Size: %d bytes, %dx%d\n", fb->len, fb->width, fb->height);

  //   uint8_t *myImage = (uint8_t *)malloc(fb->len);
  //   memcpy(myImage, fb->buf, fb->len);

  //   esp_camera_fb_return(fb);  // Always return the buffer when done!
  // }

}

// void configurCamera(int brightness, int contrast, int saturation, framesize_t size) {
//   sensor_t *s = esp_camera_sensor_get();
//   s->set_framesize(s, size);        // framesizes
//   s->set_brightness(s, brightness); // -2 to 2
//   s->set_saturation(s, 0);          // -2 to 2
//   s->set_contrast(s, contrast);     // -2 to 2
//   s->set_special_effect(s, effect); // 2 = grayscale
// }

// Then call it anywhere:
// configurCamera(1, 0, FRAMESIZE_VGA);

void loop() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "IP") {
      Serial.print("http://");
      Serial.print(WiFi.localIP());
      // Serial.println("/capture");
    }
  }

  delay(10);
}
