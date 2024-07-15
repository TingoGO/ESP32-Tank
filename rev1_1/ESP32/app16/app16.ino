/*
ver11
增加trigger_detector功能,上發條步驟跟著調整
ver12
增加初始化讀取指令8,回傳81和82指令為shot校正值
ver13
增加crash detector( pin 32),因為pin35~39沒有pull up功能,原本pin 32被 PIN_shot佔用,由於該pin只用到analogread所以改道pin 35
digitalRead閥值為1.70 V,輸入正負+-1mV都會導致誤判,esp32沒有施密特觸發
ver14
crash改用超音波sr-04偵測
ver15
shot改用digital偵測
ver16
增加當閒置超過5分鐘時,定時自動關閉ESP cam(如果有開啟的話)
增加完全unload(1,4)的指令,讓APP關閉時把trigger完全退回到鬆弛狀，不然橡皮筋會疲乏
增加初始化功能，當APP連線時會自動讓trigger到ready狀態
修正PIN_power_ESP_CAM而外串接ULN2003的硬體錯誤，這樣根本多此一舉
*/
//藍芽教學https://www.youtube.com/watch?v=aM2ktMKAunw&ab_channel=mo_thunderz
//https://www.youtube.com/watch?v=eP35zgZnQY4&ab_channel=mo_thunderz

//待辦 改成static區域變數

//Mode Select
//#define SHOT_DETECT_ANALOG
//#define DEV_MODE
//#define SHACK_AFTER_FIRE_ENABLE


#include "BluetoothSerial.h"
BluetoothSerial BT;
byte reveive_msg1 = 0;
byte reveive_msg2 = 0;
bool reveive_temp1 = 0;
bool reveive_temp2 = 0;
bool reveive_temp3 = 0;
bool trigger_motor_toggle1 = 0;  //發射toogle
bool trigger_motor_toggle2 = 0;  //上發條toggle
bool azimuth_motor_toggle = 0;
unsigned long trigger_motor_time = 0;
unsigned long azimuth_motor_toggleime = 0;
int trigger_motor_value = 0;     //發射步進馬達步數，值要是.....-8,-4,4,8....
int azimuth_motor_value = 1024;  //角度步進馬達步數
int azimuth_motor_goal = 1024;


unsigned int elevation_servo_MAX = 6552;
unsigned int elevation_servo_MIN = 3277;
int elevation_servo_setting_value = 4915;
int elevation_servo_current_value = 4915;
unsigned long elevation_servo_time = 0;
bool elevation_servo_toggle1 = 0;
bool elevation_servo_toggle2 = 0;
bool trigger_ready = 0;
bool ability_charge_toggle = 0;
byte ability_overload_toggle = 0;
int background_shot = 0;
unsigned long trigger_release_time = 0;
unsigned long shot_detect_time = 0;

unsigned long idle_begin_time = 0;



/*原始版pin
#define PIN_trigger_motor_4  16  
#define PIN_trigger_motor_1  17
#define PIN_trigger_motor_2  5
#define PIN_trigger_motor_3  18
#define PIN_azimuth_motor_4  19 
#define PIN_azimuth_motor_1  21
#define PIN_azimuth_motor_2  22
#define PIN_azimuth_motor_3  23
#define PWM_channel_elevation_servo  13
*/
/*正式版pin,方向步進馬達因為齒輪結構,所以方向要相反
#define PIN_trigger_motor_4  16  
#define PIN_trigger_motor_1  17
#define PIN_trigger_motor_2  5
#define PIN_trigger_motor_3  18
#define PIN_azimuth_motor_4  19 
#define PIN_azimuth_motor_1  23
#define PIN_azimuth_motor_2  22
#define PIN_azimuth_motor_3  21
#define PWM_channel_elevation_servo  13
*/
/*正式版pin,方向步進馬達B因為齒輪結構,所以方向要相反 20240427
#define PIN_trigger_motor_4  16   
#define PIN_trigger_motor_1  18
#define PIN_trigger_motor_2  5
#define PIN_trigger_motor_3  17
#define PIN_azimuth_motor_4  19 
#define PIN_azimuth_motor_1  23
#define PIN_azimuth_motor_2  22
#define PIN_azimuth_motor_3  21
#define PWM_channel_elevation_servo  13
*/
/*因為跟esp32 cam溝通要用掉Pin16和17,所以要更改位置
#define PIN_trigger_motor_4  14   
#define PIN_trigger_motor_1  18
#define PIN_trigger_motor_2  5
#define PIN_trigger_motor_3  25
#define PIN_azimuth_motor_4  19 
#define PIN_azimuth_motor_1  23   
#define PIN_azimuth_motor_2  22
#define PIN_azimuth_motor_3  21
#define PWM_channel_elevation_servo  13
*/
//以下是我焊錯的
#define PIN_trigger_motor_4 25  //17
#define PIN_trigger_motor_1 19  //5
#define PIN_trigger_motor_2 18  //18
#define PIN_trigger_motor_3 5   // 19
#define PIN_azimuth_motor_4 14
#define PIN_azimuth_motor_1 23
#define PIN_azimuth_motor_2 22
#define PIN_azimuth_motor_3 21
#define PWM_channel_elevation_servo 13
/*
pin 15接uln2003 會導致esp32不會輸出bootloader訊息
rst:0x1 (POWERON_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1184
load:0x40078000,len:13260

*/
//到這裡

#define RXD2 16
#define TXD2 17
#define PIN_power_ESP_CAM 15
#define MAX_WIFI_LENGTH 33
#define ESP_CAM_IDLE_TIMEOUT 300000
char wifi_SSID[MAX_WIFI_LENGTH];
char wifi_password[MAX_WIFI_LENGTH];
int wifi_input_index = 0;
bool ESPcam_initialized = 0;
bool ESPcam_on = 0;
static unsigned long cam_initial_time;

#define PIN_engineL_speed 13
#define PIN_engineL_reverse 12
#define PIN_engineR_speed 27
#define PIN_engineR_reverse 26
#define PWM_channel_engineL 15
#define PWM_channel_engineR 14
#define PWM_engine_frequency 10  //測試馬達週期，越短扭力越強但會越斗，50Hz只能打開轉速130，20Hz轉速 50，10Hz 轉速20


#define PIN_elevation_servo 4
#define PIN_shot 35
#define PIN_trigger_detector 33
#define PIN_distance_detector 32

#define ELEVATION_INCREMENT 10
#define ELEVATION_PERIOD 20  //this should be lager than 20mS
#define ELEVATION_SHAKING_VALUE_BY_FIRING 50

#define TRIGGGER_motor_fire_step_value 400
#define STEP_MOTOR_PERIOD 2000  //步進馬達在同時開啟兩線圈時,最快1600uS;慢慢加速可以到900uS(每次加速3uS)但是扭力會變幾乎0,有負載時可以加到1700us
#define CRASH_COUNT_THRESHOLD 3
#define CRASH_VALUE_THRESHOLD 1000  //about 10cm
#define SHOT_CALIBRATE_TIMES 5400   //that's about 1 second for 5400 times of analogRead
#define SHOT_COUNT_THRESHOLD 3
#define SHOT_VALUE_THRESHOLD 120  //that's about 100mV,but  see: https://deepbluembedded.com/esp32-adc-tutorial-read-analog-voltage-arduino/
#define SHOT_INVICIBLE_PERIOD_BY_FIRE 100 // typical value will be 50mS, but it depend on your trigger detector design
#define SHOT_DETECT_PERIOD 1
#define ABILITY_OVERLOAD_LOADING_PERIOD_DECREASE 300


int HP = 0;  



void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 initiliazed");
  BT.begin("ESP32_tank");  //Name of your Bluetooth interface -> will show up on your phone
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  pinMode(PIN_power_ESP_CAM, OUTPUT);
  digitalWrite(PIN_power_ESP_CAM, 1);
  pinMode(PIN_trigger_motor_4, OUTPUT);
  pinMode(PIN_trigger_motor_1, OUTPUT);
  pinMode(PIN_trigger_motor_2, OUTPUT);
  pinMode(PIN_trigger_motor_3, OUTPUT);
  pinMode(PIN_azimuth_motor_4, OUTPUT);
  pinMode(PIN_azimuth_motor_1, OUTPUT);
  pinMode(PIN_azimuth_motor_2, OUTPUT);
  pinMode(PIN_azimuth_motor_3, OUTPUT);
  pinMode(PIN_engineL_reverse, OUTPUT);
  pinMode(PIN_engineR_reverse, OUTPUT);

  ledcSetup(PWM_channel_elevation_servo, 50, 16); 
  ledcAttachPin(PIN_elevation_servo, PWM_channel_elevation_servo);

  ledcSetup(PWM_channel_engineL, PWM_engine_frequency, 8);
  ledcAttachPin(PIN_engineL_speed, PWM_channel_engineL);
  ledcSetup(PWM_channel_engineR, PWM_engine_frequency, 8);
  ledcAttachPin(PIN_engineR_speed, PWM_channel_engineR);

  pinMode(PIN_shot, INPUT);
  pinMode(PIN_trigger_detector, INPUT_PULLUP);
  pinMode(PIN_distance_detector, OUTPUT);
}

void loop() {


  if (ability_charge_toggle) {
    crashRead(&ability_charge_toggle);
  } else {
    if (millis() >= shot_detect_time + SHOT_DETECT_PERIOD) {
      shot_detect_time = millis();
      shotDetect(trigger_release_time);
    }
  }

  triggerDetectorRead(&trigger_ready, &trigger_release_time);

  // if(background_shot>0)
  // shotDetect();

  if (trigger_motor_toggle1 || trigger_motor_toggle2 || (ability_overload_toggle > 0) || (trigger_motor_value != 0)) {  //trigger_motor_toggle1發射    trigger_motor_toggle2上發條,校正
    stepMotor_fire(STEP_MOTOR_PERIOD, PIN_trigger_motor_4, PIN_trigger_motor_1, PIN_trigger_motor_2, PIN_trigger_motor_3, &trigger_motor_value, &trigger_motor_time);
  }



  if (elevation_servo_toggle1) {

    if (millis() - elevation_servo_time >= ELEVATION_PERIOD) {
      elevation_servo_time = millis();
      if (elevation_servo_setting_value - elevation_servo_current_value > ELEVATION_INCREMENT)
        ledcWrite(PWM_channel_elevation_servo, elevation_servo_current_value += ELEVATION_INCREMENT);
      else if (elevation_servo_setting_value - elevation_servo_current_value < -ELEVATION_INCREMENT)
        ledcWrite(PWM_channel_elevation_servo, elevation_servo_current_value -= ELEVATION_INCREMENT);
      else if (elevation_servo_current_value == elevation_servo_setting_value) {
        elevation_servo_toggle1 = 0;
        ledcWrite(PWM_channel_elevation_servo, 0);
      } else {
        elevation_servo_current_value = elevation_servo_setting_value;
        ledcWrite(PWM_channel_elevation_servo, elevation_servo_current_value);
      }
    }
  }
  if (elevation_servo_toggle2) {
    if (millis() - elevation_servo_time >= 300) {
      ledcWrite(PWM_channel_elevation_servo, 0);
      elevation_servo_toggle2 = 0;
    } else if (millis() - elevation_servo_time >= 200)
      ledcWrite(PWM_channel_elevation_servo, elevation_servo_setting_value);
    else if (millis() - elevation_servo_time >= 100) {
      if ((elevation_servo_setting_value - ELEVATION_SHAKING_VALUE_BY_FIRING) > elevation_servo_MIN)
        ledcWrite(PWM_channel_elevation_servo, elevation_servo_setting_value - ELEVATION_SHAKING_VALUE_BY_FIRING);
      else
        ledcWrite(PWM_channel_elevation_servo, elevation_servo_MIN);
    } else {
      if ((elevation_servo_setting_value + ELEVATION_SHAKING_VALUE_BY_FIRING) < elevation_servo_MAX)
        ledcWrite(PWM_channel_elevation_servo, elevation_servo_setting_value + ELEVATION_SHAKING_VALUE_BY_FIRING);
      else
        ledcWrite(PWM_channel_elevation_servo, elevation_servo_MAX);
    }
  }


  if (azimuth_motor_toggle) {
    stepMotor_direction();
  }

  if (ESPcam_initialized) {
    char serial2_temp = 0;
    serial2_temp = Serial2.read();
    if (serial2_temp != '$') {  //我在ESP_cam用$作為標記
      if (serial2_temp != 255)
        Serial.print(serial2_temp);  //DD
      if (millis() - cam_initial_time >= 10000) {
        BTwrite(91, 3);
        ESPcam_initialized = 0;
      }

    } else {
      // if (ESPcam_initialized) {  //?? not sure why i add this?
      Serial.print("IP = ");  //DD

      serial2_temp = Serial2.read();
      while (serial2_temp != '$') {

        BTwrite(9, serial2_temp);
        Serial.print(serial2_temp);  //DD
        serial2_temp = Serial2.read();
      }
      BTwrite(91, 1);  //告訴app可以連入ESP_cam了
      ESPcam_initialized = 0;
      ESPcam_on = 1;
      // }                        //??
    }
  }

  if (ESPcam_on) {
    if (millis() - idle_begin_time > ESP_CAM_IDLE_TIMEOUT) {
      ESPcam_on = 0;
      Serial2.write(5);
    }
  }

  if (BT.available()) {
    BTread();
    if (reveive_temp3)  //裡面擺switch，一定要在BTread程序後面，不然不會觸發
    {

      switch (reveive_msg1) {

        case 1:
          switch (reveive_msg2) {
            case 1:
              trigger_motor_value = TRIGGGER_motor_fire_step_value;
              trigger_motor_toggle1 = 1;
              break;
            case 2:
              trigger_motor_value = trigger_motor_value + 4;
              break;
            case 3:
              trigger_motor_value = trigger_motor_value - 4;
              break;
            case 4:
              trigger_motor_value = -2048;
              break;
              /*
             * 把app測試按鈕區域打開
             * case 4:              ///case4刪掉，以及app invnetor 測試按鈕
              BTwrite(6,5);
              break;
                          case 5:              ///case4刪掉
              BTwrite(56,5);
              break;
                          case 6:              
                BT.write(6);  
                BT.write(1);
                BT.write(31);
                 BT.write(32);
                  BT.write(33);
                   BT.write(34);
              break;        ///刪到這裡
             */
            default:
              TransmitError("(ESP32)error when state: ", reveive_msg1, "no such value: ", reveive_msg2);
              break;
          }

          break;
        case 8:
#ifndef DEV_MODE
          trigger_motor_value = 2048;
          trigger_motor_toggle2 = 1;
#endif
          backgroundValue();
          break;
        case 11:
          if (reveive_msg2 == 1) {
            ability_charge_toggle = 1;
            engineWrite(0, PWM_channel_engineL, PIN_engineL_reverse);
            engineWrite(0, PWM_channel_engineR, PIN_engineR_reverse);
          } else if (reveive_msg2 == 2) {
            ability_charge_toggle = 0;
            engineWrite(0, PWM_channel_engineL, PIN_engineL_reverse);
            engineWrite(0, PWM_channel_engineR, PIN_engineR_reverse);
            BTwrite(11, 3);
          } else
            TransmitError("(ESP32)error when state: ", reveive_msg1, "no such value: ", reveive_msg2);
          break;
        case 12:
          if (reveive_msg2 == 1)
            ability_overload_toggle = 4;
          else if (reveive_msg2 == 2)
            ability_overload_toggle = 1;
          else
            TransmitError("(ESP32)error when state: ", reveive_msg1, "no such value: ", reveive_msg2);
          break;
        case 21:
          azimuth_motor_toggle = 1;
          azimuth_motor_goal = 1024 + map(reveive_msg2, 0, 255, 0, 1024);
          break;
        case 22:
          azimuth_motor_toggle = 1;
          azimuth_motor_goal = 1024 - map(reveive_msg2, 0, 255, 0, 1024);
          break;
        case 23:
          if (reveive_msg2 != 1)
            TransmitError("(ESP32)error when state: ", reveive_msg1, "no such value: ", reveive_msg2);
          else
            azimuth_motor_value = 1024;
          break;
        case 31:
          if (!elevation_servo_toggle2) {
            elevation_servo_time = millis();
          }
          elevation_servo_toggle1 = 1;
          elevation_servo_setting_value = map(reveive_msg2, 0, 255, elevation_servo_MIN, elevation_servo_MAX);  //檢查1byte To 2byte變數error?
          break;
        case 33:
          if (reveive_msg2 == 1) {
            BTwrite(61, elevation_servo_setting_value / 256);
            BTwrite(62, elevation_servo_setting_value % 256);
            elevation_servo_MAX = elevation_servo_setting_value;
          } else
            TransmitError("(ESP32)error when state: ", reveive_msg1, "no such value: ", reveive_msg2);
          break;
        case 34:
          if (reveive_msg2 == 1) {
            BTwrite(63, elevation_servo_setting_value / 256);
            BTwrite(64, elevation_servo_setting_value % 256);
            elevation_servo_MIN = elevation_servo_setting_value;
          } else
            TransmitError("(ESP32)error when state: ", reveive_msg1, "no such value: ", reveive_msg2);
          break;
        case 35:
          elevation_servo_MAX = 256 * reveive_msg2;
          break;
        case 36:
          elevation_servo_MAX = elevation_servo_MAX + reveive_msg2;
          elevation_servo_current_value = (elevation_servo_MAX + elevation_servo_MIN) / 2;
          elevation_servo_setting_value = (elevation_servo_MAX + elevation_servo_MIN) / 2;
          break;
        case 37:
          elevation_servo_MIN = 256 * reveive_msg2;
          break;
        case 38:
          elevation_servo_MIN = elevation_servo_MIN + reveive_msg2;
          elevation_servo_current_value = (elevation_servo_MAX + elevation_servo_MIN) / 2;
          elevation_servo_setting_value = (elevation_servo_MAX + elevation_servo_MIN) / 2;
          break;
        case 41:
          engineWrite(reveive_msg2, PWM_channel_engineL, PIN_engineL_reverse);
          Serial.println("motorL");
          Serial.println(reveive_msg2);
          //xx=reveive_msg2;                       //delete
          break;
        case 42:
          engineWrite(-reveive_msg2, PWM_channel_engineL, PIN_engineL_reverse);
          Serial.println("motorL");
          Serial.println(-reveive_msg2);
          //xx=-reveive_msg2;                         //deleter
          break;
        case 51:
          engineWrite(reveive_msg2, PWM_channel_engineR, PIN_engineR_reverse);
          Serial.println("motorR");
          Serial.println(reveive_msg2);
          break;
        case 52:
          engineWrite(-reveive_msg2, PWM_channel_engineR, PIN_engineR_reverse);
          break;
          //    case 5:
        case 61:
          {
            cam_initial_time = millis();
            switch (reveive_msg2) {
              case 1:
                {
                  ESPcam_initialized = 0;
                  digitalWrite(PIN_power_ESP_CAM, 0);
                  delay(1);
                  digitalWrite(PIN_power_ESP_CAM, 1);
                  delay(100);

                  cam_initial_time = millis();
                  int serial2_temp = 0;
                  bool case61toggle1 = 0;
                  bool case61toggle2 = 0;
                  serial2_temp = Serial2.read();
                  while (!ESPcam_initialized) {

                    switch (serial2_temp) {  //因為ESP_cam在開機前會有雜訊所以只有在連續收到1,2,3才能確定真的開機了
                      case 1:
                        case61toggle1 = 1;
                        break;
                      case 2:
                        if (case61toggle1)
                          case61toggle2 = 1;
                        case61toggle1 = 0;
                        break;
                      case 3:
                        if (case61toggle2) {
                          ESPcam_initialized = 1;
                          Serial.println("it worksssssssss");  //DD
                        }
                        break;
                      default:
                        case61toggle1 = 0;
                        case61toggle2 = 0;
                        break;
                    }

                    if (ESPcam_initialized || millis() - cam_initial_time >= 10000)
                      break;
                    serial2_temp = Serial2.read();
                  }

                  if (!ESPcam_initialized) {
                    BTwrite(91, 2);  //沒成功開啟ESP_cam
                  }
                }
                break;
              case 2:
                wifi_input_index = 0;  //即將開始從APP輸入wif_SSID
                for (int i = 0; i < MAX_WIFI_LENGTH; i++)
                  wifi_SSID[i] = 0;
                break;
              case 3:
                wifi_input_index = 0;  //即將開始從APP輸入wif_password
                for (int i = 0; i < MAX_WIFI_LENGTH; i++)
                  wifi_password[i] = 0;
                break;
              case 4:
                Serial.print("\n(esp32)wifi_SSID");  //DD

                Serial2.write(2);
                for (int i = 0; i < MAX_WIFI_LENGTH; i++) {
                  if (wifi_SSID[i] != 0) {
                    Serial2.print(wifi_SSID[i]);
                    Serial.print(wifi_SSID[i]);  //DD
                  } else
                    break;
                }
                Serial.print("\n(esp32)nwifi_password:");
                Serial2.write(3);
                for (int i = 0; i < MAX_WIFI_LENGTH; i++) {
                  if (wifi_password[i] != 0) {
                    Serial2.print(wifi_password[i]);
                    Serial.print(wifi_password[i]);  //DD
                  } else
                    break;
                }
                Serial2.write(4);


                // if (ESPcam_initialized) {
                //   Serial.println("#####test:");  //DD
                //   char serial2_temp = 0;
                //   serial2_temp = Serial2.read();
                //   while (serial2_temp != '$') {  //我在ESP_cam用$作為標記
                //     if (serial2_temp != 255)
                //       Serial.print(serial2_temp);  //DD
                //     if (millis() - cam_initial_time >= 10000) {
                //       TransmitError("(ESP32_cam) failed to turn on WIFI when stage", 61, "value", 4);
                //       ESPcam_initialized = 0;
                //       break;
                //     }
                //     serial2_temp = Serial2.read();
                //   }
                //   if (ESPcam_initialized) {
                //     Serial.print("IP = ");  //DD

                //     serial2_temp = Serial2.read();
                //     while (serial2_temp != '$') {

                //       BTwrite(9, serial2_temp);
                //       Serial.print(serial2_temp);  //DD
                //       serial2_temp = Serial2.read();
                //     }
                //     BTwrite(91, 0);  //告訴app可以連入ESP_cam了
                //   }
                // }

                break;

              case 5:
                ESPcam_on = 0;
                Serial2.write(5);
                break;

              default:
                break;
            }
          }
          break;
        case 62:
          wifi_SSID[wifi_input_index] = reveive_msg2;
          wifi_input_index++;
          break;
        case 63:
          wifi_password[wifi_input_index] = reveive_msg2;
          wifi_input_index++;
          break;


        default:
          TransmitError("(ESP32)there is no such state: ", reveive_msg1, "value: ", reveive_msg2);


          break;
      }
      reveive_temp3 = 0;
      reveive_msg1 = 0;
      reveive_msg2 = 0;
    }
  }
}
