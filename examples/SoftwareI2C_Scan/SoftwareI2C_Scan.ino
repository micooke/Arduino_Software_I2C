#include <SoftwareI2C.h>

SoftwareI2C softwarei2c;

void setup()
{
    Serial.begin(9600);
    softwarei2c.init(0, 1); // sda, scl
    softwarei2c.begin();
}

void loop()
{
    Serial.println("Scanning...");
    uint8_t devicesFound = 0;
    for(uint8_t address=1; address<128; ++address)
    {
        if(softwarei2c.beginTransmission(address))
        {
            Serial.print("Found : 0x");
            Serial.println(address, HEX);
            ++devicesFound;
        }
        softwarei2c.endTransmission();
    }

    if (!devicesFound)
    {
      Serial.println("No I2C devices found");
    }
    delay(1000);
}