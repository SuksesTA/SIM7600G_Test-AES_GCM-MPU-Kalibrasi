#if !defined(DataHandler)
#define DataHandler

#include <Arduino.h>
#include <SPIFFS.h>
#include <cstdlib>
#include "OLED/SSD1306.h"

#define MAX_BUFFER_SIZE 256

bool SPIFFSBegin();
bool saveData(String data, String filename);
String getData(String filename);
bool randomizeMQTTTopic(int length = 6);

#endif // DataHandler
