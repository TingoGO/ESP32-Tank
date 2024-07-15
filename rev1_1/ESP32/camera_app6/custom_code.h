#include "esp_camera.h"
typedef enum Aspect_ratio {
  _4to3,
  _16to9,
  _18to9,
  _20to9,
  _21to9
} Aspect_ratio;


void image_zoom(sensor_t* s,int zoomx,Aspect_ratio _aspect_raio);

/*
21:9
672:288
*/