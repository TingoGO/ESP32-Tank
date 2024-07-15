#ifdef SHOT_DETECT_ANALOG
void shotDetect(unsigned long trigger_release_time) {

  static byte shot_count = 0;
  static char shot_sensor_value_256;
  static int shot_sensor_value;
  shot_sensor_value = analogRead(PIN_shot);
  if (shot_sensor_value > SHOT_VALUE_THRESHOLD) {
    shot_count++;
   // if (shot_sensor_value / 16 > shot_sensor_value_256)  //for test
   //   shot_sensor_value_256 = shot_sensor_value / 16;  //for test
  } else {
    if (shot_count >= SHOT_COUNT_THRESHOLD) {
      if(millis()-trigger_release_time>SHOT_INVICIBLE_PERIOD_BY_FIRE)
      //TransmitError("(ESP32)analog MAX value/16= ", shot_sensor_value_256, "count: ", shot_count);  //for test
      BTwrite(7, 1);
   //   shot_sensor_value_256 = 0;  //for test
    }
    shot_count = 0;
  }
}
#else
void shotDetect(unsigned long trigger_release_time) {
  static byte shot_count = 0;
  static bool shot_sensor_value;
  shot_sensor_value = digitalRead(PIN_shot);
  if (shot_sensor_value) {
    shot_count++;
  } else {
    if (shot_count >= SHOT_COUNT_THRESHOLD) {
      Serial.println(millis()-trigger_release_time);//DD
      if(millis()-trigger_release_time>SHOT_INVICIBLE_PERIOD_BY_FIRE)
      BTwrite(7, 1);
    }
    shot_count = 0;
  }
}
#endif

void crashRead(bool *ability_charge_toggle_) {
  static byte crash_count = 0;
  digitalWrite(PIN_distance_detector, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_distance_detector, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_distance_detector, LOW);
  pinMode(PIN_distance_detector, INPUT);
  delayMicroseconds(100);
  long duration = pulseIn(PIN_distance_detector, HIGH, 10000);  //超過10000忽略,約1.5公尺,SR04最多可以測到5公尺
  if (duration == 0)
    duration = 30000;  //max value of sr-4
  delayMicroseconds(100);
  pinMode(PIN_distance_detector, OUTPUT);
  if (duration < CRASH_VALUE_THRESHOLD) {
    crash_count++;

  } else
    crash_count = 0;
  if (crash_count == CRASH_COUNT_THRESHOLD) {
    *ability_charge_toggle_ = 0;
    delay(500);
    engineWrite(0, PWM_channel_engineL, PIN_engineL_reverse);
    engineWrite(0, PWM_channel_engineR, PIN_engineR_reverse);
    BTwrite(11, 3);
    crash_count = 0;
  }
}

void backgroundValue() {
  unsigned long temp_shot = 0;
  background_shot = 0;
  for (int i = 0; i < SHOT_CALIBRATE_TIMES; i++) {
    temp_shot += analogRead(PIN_shot);
  }
  pinMode(PIN_shot,INPUT);       //it need to be restarted as input, if it need to be digitalread, instead
  background_shot = temp_shot / SHOT_CALIBRATE_TIMES;
  BTwrite(81, background_shot / 256);
  BTwrite(82, background_shot % 256);
}