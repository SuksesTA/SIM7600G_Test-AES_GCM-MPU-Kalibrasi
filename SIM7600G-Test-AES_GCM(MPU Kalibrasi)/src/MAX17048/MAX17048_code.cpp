#include "MAX17048.h"
#include <Wire.h>

static SFE_MAX1704X gauge;

namespace MAXFG {

bool begin() {
  Wire.begin(21, 22);           
  return gauge.begin();         
}

float getSOC()     { return gauge.getSOC();     }
float getVoltage() { return gauge.getVoltage(); }

} // namespace MAXFG
