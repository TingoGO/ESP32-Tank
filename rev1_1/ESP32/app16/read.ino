void triggerDetectorRead(bool *status, unsigned long *trigger_release_time) {
  static byte detector_count = 0;
  static unsigned int detector_time = 0;
  static unsigned int detector_temp = 0;
  static unsigned int detector_value = 0;

  if (!*status) {
    detector_value = !digitalRead(PIN_trigger_detector);
    if (detector_value)
      *status = 1;
    detector_time = millis();
  } else {
    detector_temp = millis();
    if (detector_temp - detector_time > 1) {
      detector_time = detector_temp;
      if (detector_value) {
        detector_value = !digitalRead(PIN_trigger_detector);
          if (!detector_value)
            *trigger_release_time = millis();
        
      } else {
        detector_value = !digitalRead(PIN_trigger_detector);
      }
      if (!detector_value) {
        detector_count++;
      } else
        detector_count = 0;

      if (detector_count > 200) {
        *status = 0;
      }
    }
  }
}