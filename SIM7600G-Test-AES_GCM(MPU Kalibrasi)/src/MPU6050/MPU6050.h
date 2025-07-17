#ifndef MPU6050_H
#define MPU6050_H
#define EEPROM_SIZE 32
#define EEPROM_ADDR_Y_OFFSET 0

#include <EEPROM.h>
#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

typedef struct {
    struct {
        float x;
        float y;
        float z;
    }gyro,accel;
} mpuReading;

// Function declarations
void mpuBegin();
mpuReading getMPU();

#endif // MPU6050_H
