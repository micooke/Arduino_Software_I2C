#define USE_SOFTWAREI2C
#ifdef USE_SOFTWAREI2C
#include <SoftwareI2C.h>  // https://github.com/micooke/SoftwareI2C
SoftwareI2C _i2c;
#else
#include <Wire.h>
#define _i2c Wire
#endif

#include <i2c_device_list.h> //adds 7952 bytes, so comment it out if you dont need it

void setup()
{
    Serial.begin(9600);
    Serial.println(__FILE__);
    Serial.println(__TIME__);
    #ifdef USE_SOFTWAREI2C
    Serial.println(">> Using SoftwareI2C");
    _i2c.init(7, 8); // sda, scl
    #else
    Serial.println(">> Using Wire");
    #endif
    _i2c.begin();
}

void loop()
{
    Serial.println("Scanning...");
    uint8_t devicesFound = 0;
    for(uint8_t address=1; address<128; ++address)
    {
        _i2c.beginTransmission(address);
        _i2c.write((uint8_t)0); // register 0
        _i2c.endTransmission();
        uint8_t device_found = (_i2c.requestFrom(address, 1) == 1); // read 1 byte of data
        _i2c.endTransmission();

        if(device_found)
        {
            Serial.print("Found : 0x");
            if (address < 16) Serial.print("0");
            Serial.println(address, HEX);
            #ifdef I2C_DEVICE_LIST_H
            String device_name = i2c_device_list(address);
            Serial.println(device_name.c_str());
            #endif
            ++devicesFound;
        }
    }

    if (!devicesFound)
    {
      Serial.println("No I2C devices found");
    }
    delay(1000);
}