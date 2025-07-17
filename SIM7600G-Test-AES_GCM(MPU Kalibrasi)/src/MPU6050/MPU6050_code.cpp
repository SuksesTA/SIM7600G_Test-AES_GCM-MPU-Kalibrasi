#include "MPU6050.h"
#include <EEPROM.h>

#define EEPROM_SIZE 64
#define EEPROM_ADDR_X_OFFSET 0
#define EEPROM_ADDR_Y_OFFSET 4
#define EEPROM_ADDR_Z_OFFSET 8

Adafruit_MPU6050 mpu;

void mpuBegin() {
    if (!mpu.begin()) {
        Serial.println("MPU6050 tidak terdeteksi!");
        return;
    }

    EEPROM.begin(EEPROM_SIZE);

    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
}

/* === Kalibrasi sumbu XYZ dan penyimpanan EEPROM === */
bool offsetReady = false;
const int calibSampleCount = 100;
int calibIndex = 0;

float xCalibSum = 0.0, yCalibSum = 0.0, zCalibSum = 0.0;
float xOffset = 0.0, yOffset = 0.0, zOffset = 0.0;

float readFloatFromEEPROM(int addr) {
    float val;
    EEPROM.get(addr, val);
    return (isnan(val) || val < -100.0 || val > 100.0) ? 0.0 : val;
}

void writeFloatToEEPROM(int addr, float val) {
    EEPROM.put(addr, val);
    EEPROM.commit();
}

mpuReading getMPU() {
    static unsigned long lastTime = millis();
    unsigned long currentTime = millis();
    float deltaTime = (currentTime - lastTime) / 1000.0;
    lastTime = currentTime;

    mpuReading reading = {};
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    float ax = a.acceleration.x;
    float ay = a.acceleration.y;
    float az = a.acceleration.z;

    // ===== Kalibrasi 100 sample pertama untuk X, Y, Z =====
    if (!offsetReady && calibIndex < calibSampleCount) {
        if (!isnan(ax) && !isnan(ay) && !isnan(az)) {
            xCalibSum += ax;
            yCalibSum += ay;
            zCalibSum += az;
            calibIndex++;
        }

        if (calibIndex == calibSampleCount) {
            float newXOffset = xCalibSum / calibSampleCount;
            float newYOffset = yCalibSum / calibSampleCount;
            float newZOffset = zCalibSum / calibSampleCount;

            // Validasi
            newXOffset = (isnan(newXOffset)) ? 0.0 : newXOffset;
            newYOffset = (isnan(newYOffset)) ? 0.0 : newYOffset;
            newZOffset = (isnan(newZOffset)) ? 0.0 : newZOffset;

            float storedX = readFloatFromEEPROM(EEPROM_ADDR_X_OFFSET);
            float storedY = readFloatFromEEPROM(EEPROM_ADDR_Y_OFFSET);
            float storedZ = readFloatFromEEPROM(EEPROM_ADDR_Z_OFFSET);

            if (abs(newXOffset - storedX) > 0.05) {
                writeFloatToEEPROM(EEPROM_ADDR_X_OFFSET, newXOffset);
                Serial.println("[EEPROM] Kalibrasi X baru disimpan");
            } else newXOffset = storedX;

            if (abs(newYOffset - storedY) > 0.05) {
                writeFloatToEEPROM(EEPROM_ADDR_Y_OFFSET, newYOffset);
                Serial.println("[EEPROM] Kalibrasi Y baru disimpan");
            } else newYOffset = storedY;

            if (abs(newZOffset - storedZ) > 0.05) {
                writeFloatToEEPROM(EEPROM_ADDR_Z_OFFSET, newZOffset);
                Serial.println("[EEPROM] Kalibrasi Z baru disimpan");
            } else newZOffset = storedZ;

            xOffset = newXOffset;
            yOffset = newYOffset;
            zOffset = newZOffset;
            offsetReady = true;

            Serial.printf("[MPU] Offset X: %.4f | Y: %.4f | Z: %.4f\n", xOffset, yOffset, zOffset);
        }
    }

    if (offsetReady) {
        ax -= xOffset;
        ay -= yOffset;
        az -= zOffset;
    }

    float vx = ax * deltaTime;
    float vy = ay * deltaTime;
    float vz = az * deltaTime;

    reading.accel.x = abs(vx);
    reading.accel.y = abs(vy);
    reading.accel.z = abs(vz);

    return reading;
}