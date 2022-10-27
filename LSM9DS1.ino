#include "Adafruit_LSM9DS1.h"
#include "Adafruit_BMP280.h"

#include <Wire.h>

Adafruit_LSM9DS1 imu;
Adafruit_BMP280 bmp; // I2C
Adafruit_Sensor *bmp_temp = bmp.getTemperatureSensor();
Adafruit_Sensor *bmp_pressure = bmp.getPressureSensor();

sensors_event_t accel, mag, gyro, temp;
sensors_event_t temp_event, pressure_event;

#define BMP280 0x77 // I2C of BMP280
char string_temp[32];
char write_buffer[256];

unsigned long prev_micros = 0;
void setup() {
    //Serial.begin(9600);
    // Begin I2c
    Wire.begin();
    // Begin LSM9DS1
    imu.begin();
    imu.setupAccel(imu.LSM9DS1_ACCELRANGE_2G);
    imu.setupMag(imu.LSM9DS1_MAGGAIN_4GAUSS);
    imu.setupGyro(imu.LSM9DS1_GYROSCALE_245DPS);
    // Begin BMP820
    bmp.begin();
    // Settings for BMP280
    bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                    Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                    Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                    Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                    Adafruit_BMP280::STANDBY_MS_250); /* Standby time. */
}
 

void loop() {
    // Read Data from IMU
    imu.getEvent(&accel, &mag, &gyro, &temp);

    // Build Accel String
    dtostrf(accel.acceleration.x,6,4,string_temp);
    sprintf(write_buffer, "%c %s",'A',string_temp);
    dtostrf(accel.acceleration.y,6,4,string_temp);
    sprintf(write_buffer, "%s %s",write_buffer,string_temp);
    dtostrf(accel.acceleration.z,6,4,string_temp);
    sprintf(write_buffer, "%s %s",write_buffer,string_temp);
    // Write Accel String to I2C
    //Serial.println(write_buffer);
    Wire.beginTransmission(0x1A);
    Wire.write(write_buffer);
    Wire.endTransmission(true);

    // Build Gyro String
    dtostrf(gyro.gyro.x,6,4,string_temp);
    sprintf(write_buffer, "%c %s",'G',string_temp);
    dtostrf(gyro.gyro.y,6,4,string_temp);
    sprintf(write_buffer, "%s %s",write_buffer,string_temp);
    dtostrf(gyro.gyro.z,6,4,string_temp);
    sprintf(write_buffer, "%s %s",write_buffer,string_temp);
    // Write Gyro String to I2C
    Wire.beginTransmission(0x1A);
    Wire.write(write_buffer);
    Wire.endTransmission(true);

    // Build Magno String
    dtostrf(mag.magnetic.x,6,4,string_temp);
    sprintf(write_buffer, "%c %s",'M',string_temp);
    dtostrf(mag.magnetic.y,6,4,string_temp);
    sprintf(write_buffer, "%s %s",write_buffer,string_temp);
    dtostrf(mag.magnetic.z,6,4,string_temp);
    sprintf(write_buffer, "%s %s",write_buffer,string_temp);
    
    // Write Magno String to I2C
    Wire.beginTransmission(0x1A);
    Wire.write(write_buffer);
    Wire.endTransmission(true);

    // get Data from BMP280
    bmp_temp->getEvent(&temp_event);
    bmp_pressure->getEvent(&pressure_event);
    
    // Build BMP280 String & IMU Temp
    dtostrf(temp_event.temperature,5,2,string_temp);
    sprintf(write_buffer, "%c %s",'P',string_temp);
    dtostrf(pressure_event.pressure,6,3,string_temp);
    sprintf(write_buffer, "%s %s",write_buffer,string_temp);
    dtostrf(micros() - prev_micros,7,1, string_temp);
    prev_micros = micros();
    sprintf(write_buffer, "%s %s",write_buffer,string_temp);
    dtostrf(temp.temperature * 0.02 + 25.0, 5,2, string_temp);
    sprintf(write_buffer, "%s %s",write_buffer,string_temp);
    // Write BMP280 String to I2C
    //Serial.println(write_buffer);
    Wire.beginTransmission(0x1A);
    Wire.write(write_buffer);
    Wire.endTransmission(true);
    delay(10);
}
