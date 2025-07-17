#pragma once
#include <SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library.h>

namespace MAXFG {
  bool begin();
  float getSOC();
  float getVoltage();
}
