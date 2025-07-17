#include <Arduino.h>
#include "SIM7600G/SIM7600G.h"
#include "OLED/SSD1306.h"
#include "DHT/DHT22.h"
#include "Data/DataHandler.h"
#include "MPU6050/MPU6050.h"
#include "MAX17048/MAX17048.h"

// ───── Encryption helper (uses aes_gcm under the hood) ─────
#include "Enkripsi/payload.h"
/* --------------------------------------------------------- */

/* ——— Example 128‑bit key (replace in production) ——— */
static const uint8_t AES_KEY[crypto::AES128_KEY_SIZE] PROGMEM = {
    0x1e,0xf2,0xa6,0x87,0x2a,0x1d,0xa1,0xf3,0x09,0xc6,0xe9,0x85,0x9e,0xb8,0x7c,0x64};

//AMtjeB:
//0x60,0x3d,0xeb,0x10,0x15,0xca,0x71,0xbe,0x2b,0x73,0xae,0xf0,0x85,0x7d,0x77,0x81

//BNtjEO:
//0x1e,0xf2,0xa6,0x87,0x2a,0x1d,0xa1,0xf3,0x09,0xc6,0xe9,0x85,0x9e,0xb8,0x7c,0x64

//YzMtje:
//0x19,0x17,0x9b,0x0f,0x67,0xee,0x01,0x08,0xa6,0xca,0x73,0x42,0xe2,0xd3,0xba,0xd7

//zMtjEO:
//0xfc,0x19,0x68,0xfe,0x2a,0x42,0xd8,0x45,0x39,0xa8,0xf7,0x55,0x75,0x0c,0xcd,0xa0

//FPuJRV:
//0xa3,0xf2,0x78,0x5b,0x6f,0x00,0x4b,0xad,0x80,0xf3,0xb6,0x76,0x1a,0x26,0x38,0x9e
/* --------------------------------------------------- */

unsigned long mainStartTime = 0;

void setup()
{
  mainStartTime = millis();
  Serial.begin(115200);

  oledBegin();
  SIM7600Gbegin();
  SPIFFSBegin();
  dhtBegin();
  mpuBegin();

  Wire.begin(21, 22);     
  if (!MAXFG::begin()) {
    Serial.println(F("MAX17048 tidak terdeteksi!"));
  }

  clearScreen();
  header(getData("topic.txt"), false);
}

void loop()
{
  gpsReading gps = getGPS();
  dhtReading dht = getDHT();
  mpuReading mpu = getMPU();

  String speedStr = String(gps.speed, 2);
  float soc = MAXFG::getSOC();
  String socStr = String(soc, 0);

  /* ===== OLED Handling ===== */
  if (gps.latitude != "0.00" && gps.longitude != "0.00") {
    header(getData("topic.txt"), true);
  } else {
    header(getData("topic.txt"), false);
    String saved = getData("gps.txt");
    gps.latitude  = splitString(saved, ',', 0);
    gps.longitude = splitString(saved, ',', 1);
  }

  gpsDisplay(String(gps.latitude), String(gps.longitude));
  dhtDisplay(String(dht.temperature), String(dht.humidity));
  mpuDisplay(String(mpu.accel.x), String(mpu.accel.y), String(mpu.accel.z));
  speedDisplay(speedStr);

  if (millis() - mainStartTime > 30000) {
    
    String timestamp = gps.timestamp.length() ? gps.timestamp : "00-00-0000 00:00:00";
    String topic = getData("topic.txt");

    char plain[256];
    snprintf(plain, sizeof(plain), "%s,%s,%s,%s,%.2f,%.2f,%.2f,%.2f,%.2f,%s",
            timestamp.c_str(), socStr.c_str(),
            gps.latitude.c_str(), gps.longitude.c_str(),
            dht.temperature, dht.humidity,
            mpu.accel.x, mpu.accel.y, mpu.accel.z,
            speedStr.c_str());

    String payloadB64;
    if (payload::encryptB64(String(plain), AES_KEY, payloadB64)) {
        publish(payloadB64);
        Serial.printf("[MQTT] AES‑GCM payload sent (%u B64 chars)\n", payloadB64.length());
    } else {
        Serial.println(F("[ENC] Encrypt failed!"));
    }
    //publish(plain);
    mainStartTime = millis();
  }
}