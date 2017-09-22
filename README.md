### SoftwareI2C

This is an Arduino software (master) I2C Library, written by @Seeed-Studio and modified by @micooke to provide a closer interface to hardware Wire libraries.

**SDA and SCL pin allocation are set using** ```begin(pinSda, pinScl);```

This library includes:
```c++
    void begin(uint8_t pinSda = 0, uint8_t pinScl = 1); 
    uint8_t beginTransmission(uint8_t addr);
    uint8_t endTransmission(bool stopBit = false);
    void end();

    uint8_t write(uint8_t dta);
    uint8_t write(uint8_t *dta, uint8_t len);
    uint8_t requestFrom(uint8_t addr, uint8_t len);
    uint8_t read();
    uint8_t available();
```

