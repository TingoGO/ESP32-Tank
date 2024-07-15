#include "custom_code.h"
#include <cmath>
void image_zoom(sensor_t* s, int zoomx, Aspect_ratio _aspect_raio) {
  switch (_aspect_raio) {
    case _4to3:
      s->set_res_raw(s, 0, 0, 0, 0, std::floor(4 * 7 * zoomx / 2), std::floor(3 * 7 * zoomx / 2), 1600 - zoomx * 4 * 7, 1200 - zoomx * 3 * 7, 480, 360, true, true);  //倒數第四個數值要能被16整，不然圖形會怪怪的
      break;
    case _16to9:
      s->set_res_raw(s, 0, 0, 0, 0, std::floor(16 * 1.5 * zoomx / 2), std::floor(9 * 1.5 * zoomx / 2), 1600 - std::floor(zoomx * 16 * 1.5), 900 - std::floor(zoomx * 9 * 1.5), 640, 360, true, true);
      break;
    case _18to9:
      s->set_res_raw(s, 0, 0, 0, 0, 8 + std::floor(18 * zoomx / 2), 204 + std::floor(9 * zoomx / 2), 1584 - zoomx * 18, 792 - zoomx * 9, 720, 360, true, true);
      break;
    case _20to9:
      s->set_res_raw(s, 0, 0, 0, 0, std::floor(20 * zoomx / 2), 240 + std::floor(9 * zoomx / 2), 1600 - zoomx * 20, 720 - zoomx * 9, 720, 324, true, true);
      break;
    case _21to9:
      s->set_res_raw(s, 0, 0, 0, 0, 2 + std::floor(21 * zoomx / 2), 258 + std::floor(9 * zoomx / 2), 1596 - zoomx * 21, 684 - zoomx * 9, 672, 288, true, true);
      break;
  }
}