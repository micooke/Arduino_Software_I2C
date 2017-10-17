#ifdef USE_SOFTWAREI2C
#include <SoftwareI2C.h>  // https://github.com/micooke/SoftwareI2C
SoftwareI2C _i2c;
#else
#include <Wire.h>
#define _i2c Wire
#endif

void setup()
{
    Serial.begin(9600);
    Serial.println(__FILE__);
    Serial.println(__TIME__);
    _i2c.init(7, 8); // sda, scl
    _i2c.begin();
}

void loop()
{
    Serial.println("Scanning...");
    uint8_t devicesFound = 0;
    for(uint8_t address=1; address<128; ++address)
    {
        _i2c.beginTransmission(address)
        uint8_t device_found = (_i2c.endTransmission() == 0);

        if(device_found)
        {
            Serial.print("Found : 0x");
            Serial.println(address, HEX);
            ++devicesFound;
        }
    }

    if (!devicesFound)
    {
      Serial.println("No I2C devices found");
    }
    delay(1000);
}