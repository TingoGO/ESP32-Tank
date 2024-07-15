//#include "esp_camera.h"  //move this to custom_code.h, Tingo
#include <WiFi.h>
#include "esp_wifi.h"

//
// WARNING!!! PSRAM IC required for UXGA resolution and high JPEG quality
//            Ensure ESP32 Wrover Module or other board with PSRAM is selected
//            Partial images will be transmitted if image exceeds buffer size
//
//            You must select partition scheme from the board menu that has at least 3MB APP space.
//            Face Recognition is DISABLED for ESP32 and ESP32-S2, because it takes up from 15
//            seconds to process single frame. Face Detection is ENABLED if PSRAM is enabled as well

// ===================
// Select camera model
// ===================
//#define CAMERA_MODEL_WROVER_KIT // Has PSRAM
//#define CAMERA_MODEL_ESP_EYE // Has PSRAM
//#define CAMERA_MODEL_ESP32S3_EYE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_PSRAM // Has PSRAM
//#define CAMERA_MODEL_M5STACK_V2_PSRAM // M5Camera version B Has PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_ESP32CAM // No PSRAM
//#define CAMERA_MODEL_M5STACK_UNITCAM // No PSRAM
#define CAMERA_MODEL_AI_THINKER  // Has PSRAM
//#define CAMERA_MODEL_TTGO_T_JOURNAL // No PSRAM
// ** Espressif Internal Boards **
//#define CAMERA_MODEL_ESP32_CAM_BOARD
//#define CAMERA_MODEL_ESP32S2_CAM_BOARD
//#define CAMERA_MODEL_ESP32S3_CAM_LCD

#include "camera_pins.h"
//add this
#include "custom_code.h"

Aspect_ratio aspect_ratio = _21to9;
const int wakeupPin = 2;          // GPIO 4 for external wake-up
RTC_DATA_ATTR int bootCount = 0;  // Number of reboots
#define PIN_BUILDIN_LED 33

sensor_t* s;
#define MAX_WIFI_LENGTH 33
char wifi_SSID[MAX_WIFI_LENGTH];
char wifi_password[MAX_WIFI_LENGTH];
int wifi_input_index = 0;

// ===========================
// Enter your WiFi credentials
// ===========================
char* ssid;
char* password;
//to here

void startCameraServer();
void setupLedFlash(int pin);

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  bootCount++;

  //add this

  pinMode(wakeupPin, INPUT_PULLUP);                          // Declaring the pin with the push button as INPUT_PULLUP
  esp_sleep_enable_ext0_wakeup((gpio_num_t)wakeupPin, LOW);  // Configure external wake-up
  if (bootCount == 1) {
    Serial.print("bootCount = ");
    Serial.println(bootCount);
    Serial.println("initial sleep");
    esp_wifi_stop();
    digitalWrite(PIN_BUILDIN_LED, 0);
    esp_deep_sleep_start();  // Enter deep sleep mode
  }
  pinMode(PIN_BUILDIN_LED, OUTPUT);
  digitalWrite(PIN_BUILDIN_LED, 1);
  delay(20);  //wait ESP32 control board to be stable
  Serial.write(1);
  Serial.write(2);
  Serial.write(3);
  int ii = 0;
  while (Serial.read() != 2) {
    Serial.println(ii);  //DD
    ii++;                //DD
    delay(100);          //DD
  }
  Serial.println("(ESP_cam)ok2");  //DD
  for (int i = 0; i < MAX_WIFI_LENGTH; i++) {
    wifi_SSID[i] = 0;
    wifi_password[i] = 0;
  }
  char serial_temp = Serial.read();
  int i = 0;
  while (serial_temp != 3) {

    wifi_SSID[i] = serial_temp;
    serial_temp = Serial.read();
    i++;
  }
  //int ssid_length = i++;
  Serial.println("(ESP_cam)ok3");  //DD
  serial_temp = Serial.read();
  i = 0;
  while (serial_temp != 4) {

    wifi_password[i] = serial_temp;
    serial_temp = Serial.read();
    i++;
  }
  //int password_length = i++;
  ssid = wifi_SSID;
  password = wifi_password;

  Serial.println("(ESP_cam)ok4");  //DD
  Serial.print("\n(ESP_cam)SSID=");
  for (int i = 0; i < MAX_WIFI_LENGTH; i++) {
    Serial.print(ssid[i]);
  }
  Serial.print("\n(ESP_cam)password=");
  for (int i = 0; i < MAX_WIFI_LENGTH; i++) {
    Serial.print(password[i]);
  }





  //to here




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
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
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

  s = esp_camera_sensor_get();
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
  setupLedFlash(LED_GPIO_NUM);
#endif

  //standby power consymption at here is 0.6W,by Tingo



  WiFi.begin(ssid, password);
  WiFi.setSleep(false);
  //add this from here
  unsigned long connect_start_time = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - connect_start_time < 10000) {
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("(ESP_cam)bootCount = ");
    Serial.println(bootCount);
    Serial.println("(ESP_cam)start to deep sleep now.");  // Print a statement before entering deep sleep
    esp_wifi_stop();
    digitalWrite(PIN_BUILDIN_LED, 0);
    esp_deep_sleep_start();  // Enter deep sleep mode
  }
  Serial.println("");
  Serial.println("WiFi connected");


  //to here

  //standby power consymption at here is 0.9W ,by Tingo



  startCameraServer();
  digitalWrite(PIN_BUILDIN_LED, 0);  //this probably need to be after startCameraServer(), or it will not acitivate?  by Tingo 2024/07/08
  //standby power consymption at here is 1.07W ,by Tingo

  //add here
  Serial.print("$");
  Serial.print(WiFi.localIP());
  Serial.println("$");
  s->set_vflip(s, 1);    //上下顛倒
  s->set_hmirror(s, 1);  //左右顛倒，所以總共旋轉180度

  //to here
#define INITIAL_RESOLUTION_VAL 20
  //"output" resolution can't below 400*300,or the output image will be cropped.
  /*
  static int set_res_raw(sensor_t *sensor, int startX, int startY, int endX, int endY, int offsetX, int offsetY, int totalX, int totalY, int outputX, int outputY, bool scale, bool binning)
{
    return set_window(sensor, (ov2640_sensor_mode_t)startX, offsetX, offsetY, totalX, totalY, outputX, outputY);
}*/
}

int xa;
int xb;
int ya;
int yb;
void loop() {

  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == 5) {
      Serial.print("(ESP_cam)bootCount = ");
      Serial.println(bootCount);
      Serial.println("(ESP_cam)start to deep sleep now.");  // Print a statement before entering deep sleep
      esp_wifi_stop();
      digitalWrite(PIN_BUILDIN_LED, 0);
      esp_deep_sleep_start();  // Enter deep sleep mode
    }
  }
  delay(10);
}