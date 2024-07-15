#define MAX_STEP_VALUE 2048

void stopStepMotor(int motorPin_1, int motorPin_2, int motorPin_3, int motorPin_4) {
  digitalWrite(motorPin_1, LOW);
  digitalWrite(motorPin_2, LOW);
  digitalWrite(motorPin_3, LOW);
  digitalWrite(motorPin_4, LOW);
}




void RunStepMotor2(int motorPin_1, int motorPin_2, int motorPin_3, int motorPin_4, int value) {
  int state = value % 4;
  if (state < 0)
    state = state + 4;  //轉換負數為正值
  switch (state) {
    case 1:
      digitalWrite(motorPin_1, HIGH);
      digitalWrite(motorPin_2, HIGH);
      digitalWrite(motorPin_3, LOW);
      digitalWrite(motorPin_4, LOW);


      break;
    case 2:
      digitalWrite(motorPin_1, LOW);
      digitalWrite(motorPin_2, HIGH);
      digitalWrite(motorPin_3, HIGH);
      digitalWrite(motorPin_4, LOW);


      break;
    case 3:
      digitalWrite(motorPin_1, LOW);
      digitalWrite(motorPin_2, LOW);
      digitalWrite(motorPin_3, HIGH);
      digitalWrite(motorPin_4, HIGH);


      break;
    case 0:
      digitalWrite(motorPin_1, HIGH);
      digitalWrite(motorPin_2, LOW);
      digitalWrite(motorPin_3, LOW);
      digitalWrite(motorPin_4, HIGH);

      break;
  }
}


void stepMotor_fire(int periodMicros, int motorPin_1, int motorPin_2, int motorPin_3, int motorPin_4, int *value, unsigned long *stepTime) {
  unsigned long tempTime = micros();
  static unsigned long step_motor_period = periodMicros;
  if (!trigger_motor_toggle1 && !trigger_motor_toggle2 && (ability_overload_toggle > 0)) {     //初始化overload該次發射
    *value = TRIGGGER_motor_fire_step_value;
    trigger_motor_toggle1 = 1;
  }


  if (tempTime - *stepTime > step_motor_period) {    //移動step motor
    if (ability_overload_toggle > 0) {               //逐漸加速轉速
      if (MAX_STEP_VALUE - *value < ABILITY_OVERLOAD_LOADING_PERIOD_DECREASE)
        step_motor_period -= 1;
    }
    if (*value > 0) {
      (*value)--;
      *stepTime = tempTime;
      RunStepMotor2(motorPin_1, motorPin_2, motorPin_3, motorPin_4, *value);

      if (trigger_motor_toggle1 && !trigger_ready) {
        if (*value % 4 == 0) {
          *value = 0;
          #ifdef SHACK_AFTER_FIRE_ENABLE
          elevation_servo_toggle2 = 1;
          #endif
          elevation_servo_time = millis();
        }
      }
      if (trigger_motor_toggle2 && trigger_ready) {
        if (*value % 4 == 0) {
          if (*value > 500)
            TransmitError("(ESP32)Trigger detect with abnormal value when loading, with step value(*0.125) = ", *value / 8, "trigger signal = ", !digitalRead(PIN_trigger_detector));  //this might happen, when trigger detector detactach before the trigger actually being releasing,then causing loading end earlier
          *value = 0;
        }
      }
    } else if (*value == 0) {
      if (trigger_motor_toggle2) {
        step_motor_period = periodMicros;
        if (ability_overload_toggle > 0) {
          ability_overload_toggle--;
          if (ability_overload_toggle == 0)
            BTwrite(12, 3);
        }
        if (!trigger_ready)
          TransmitError("(ESP32)Fail to detect trigger when loading , trigger_ready = ", trigger_ready, "while trigger signal = ", !digitalRead(PIN_trigger_detector));
      }
      stopStepMotor(motorPin_1, motorPin_2, motorPin_3, motorPin_4);  //觸發上發條以及回傳發條Ready指令
      trigger_motor_toggle2 = 0;
      if (tempTime - *stepTime > 10000)  //等0.01秒後上發條
      {
        if (trigger_motor_toggle1) {
          if (trigger_motor_toggle1 && trigger_ready)
            TransmitError("(ESP32)Fail to detect trigger when shooting , trigger_ready = ", trigger_ready, "while trigger signal = ", !digitalRead(PIN_trigger_detector));
          trigger_motor_toggle2 = 1;
          *value = MAX_STEP_VALUE;
          *stepTime = tempTime;
        }
        trigger_motor_toggle1 = 0;
      }

      if (!trigger_motor_toggle1 && !trigger_motor_toggle2)
        BTwrite(6, 1);


    } else {
      (*value)++;
      RunStepMotor2(motorPin_1, motorPin_2, motorPin_3, motorPin_4, *value);
      *stepTime = tempTime;
    }
  }
}

void stepMotor_direction() {
  unsigned long tempTime = micros();
  if (tempTime - azimuth_motor_toggleime > 3000)  //方位步進馬達速度
  {
    if (azimuth_motor_goal > azimuth_motor_value) {
      azimuth_motor_value++;
      RunStepMotor2(PIN_azimuth_motor_4, PIN_azimuth_motor_1, PIN_azimuth_motor_2, PIN_azimuth_motor_3, azimuth_motor_value);
    } else {
      if (azimuth_motor_goal < azimuth_motor_value) {
        azimuth_motor_value--;
        RunStepMotor2(PIN_azimuth_motor_4, PIN_azimuth_motor_1, PIN_azimuth_motor_2, PIN_azimuth_motor_3, azimuth_motor_value);
      } else {
        azimuth_motor_toggle = 0;
        stopStepMotor(PIN_azimuth_motor_4, PIN_azimuth_motor_1, PIN_azimuth_motor_2, PIN_azimuth_motor_3);
      }
    }
    azimuth_motor_toggleime = tempTime;
  }
}


/*
 * 
 * void RunStepMotor2(int periodMicros,int motorPin_1,int motorPin_2,int motorPin_3,int motorPin_4,int value)
{

  byte state=value%4;
  switch(state)
  {
    case 1:
    digitalWrite(motorPin_1, HIGH);
    digitalWrite(motorPin_2,HIGH);
    digitalWrite(motorPin_3, LOW);
    digitalWrite(motorPin_4,LOW);
    delayMicroseconds(periodMicros);
    break;
    case 2:
    digitalWrite(motorPin_1, LOW);
    digitalWrite(motorPin_2,HIGH);
    digitalWrite(motorPin_3, HIGH);
    digitalWrite(motorPin_4,LOW);
    delayMicroseconds(periodMicros);
    break;
    case 3:
    digitalWrite(motorPin_1, LOW);
    digitalWrite(motorPin_2,LOW);
    digitalWrite(motorPin_3, HIGH);
    digitalWrite(motorPin_4,HIGH);
    delayMicroseconds(periodMicros);
    break;
    case 0:
    digitalWrite(motorPin_1, HIGH);
    digitalWrite(motorPin_2,LOW);
    digitalWrite(motorPin_3, LOW);
    digitalWrite(motorPin_4,HIGH);
    delayMicroseconds(periodMicros);
    break;

  }
}
*/
