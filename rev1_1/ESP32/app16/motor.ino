void engineWrite(int motorTemp, byte motChannel, byte pin_mot_direction) {
  if (ability_charge_toggle) {
    digitalWrite(pin_mot_direction, 0);
    if (motorTemp < 0)
      ledcWrite(motChannel, 255 + motorTemp);
    else
      ledcWrite(motChannel, 255);
  } else {
    if (motorTemp > 0) {
      // Serial.println(motorTemp);
      digitalWrite(pin_mot_direction, 0);
      ledcWrite(motChannel, motorTemp);
    } else {
      digitalWrite(pin_mot_direction, 1);
      // Serial.println(motorTemp);
      ledcWrite(motChannel, 255 + motorTemp);
    }
  }
}
