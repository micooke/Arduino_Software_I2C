#include <SoftwareI2C.h>  // https://github.com/micooke/SoftwareI2C
#include <SerialCommand.h> // https://github.com/kroimon/Arduino-SerialCommand
#include <Vector.h> // https://github.com/zacsketches/Arduino_Vector

SerialCommand sCmd;

void unrecognized(const char *c)
{
   Serial.println(
       "Commands are :\n"
       "P sda_pin scl_pin - sets I2C pins\n"
       "S - scan for I2C devices using the current sda,scl pins\n"
       "R device_address reg_num reg_len - reads 'reg_len' bytes from register "
       "'reg_num'\n"
       "RX device_address reg_num reg_len - (expects HEX values) reads 'reg_len' bytes from register "
       "'reg_num'\n"
       "W device_address reg_num reg_val - writes 'reg_val' to register "
       "'reg_num'\n"
       "WX device_address reg_num reg_val - (expects HEX values) writes 'reg_val' to register "
       "'reg_num'\n");
   Serial.println("Note : For RX and WX drop the preceeding 0x from the hex string");
   Serial.println("Example: RX 40 0 1 -> I2C address = 0x40, register number = 0, number of registers to read = 1");
}

bool readRegister(uint8_t deviceAddress, uint8_t reg, uint8_t len, Vector<uint8_t> &inBytes)
{
   bool ack = true;

   softwarei2c.beginTransmission(deviceAddress);
   ack &= softwarei2c.write(reg);
   softwarei2c.endTransmission();  // false);
   ack &= (softwarei2c.requestFrom(deviceAddress, len) > 0);
   
   if (ack)
   {
     while (softwarei2c.available())
     {
        inBytes.push_back(softwarei2c.read());
     }
   }
   return ack;
}

bool writeRegister(uint8_t deviceAddress, uint8_t reg, uint8_t data)
{
   bool ack = true;
   softwarei2c.beginTransmission(deviceAddress);
   ack &= softwarei2c.write(reg);
   ack &= softwarei2c.write(data);
   softwarei2c.endTransmission();  // true);

   return ack;
}

void readCommand(uint8_t i2c_address, uint8_t reg_num, uint8_t reg_len)
{
   Vector<uint8_t> inBytes;
   bool success = readRegister(i2c_address, reg_num, reg_len, inBytes);
   Serial.print(success ? "SUCCESS - 0x" : "FAILED - 0x");
   Serial.print(i2c_address, HEX);
   Serial.print(" : read ");
   Serial.print(reg_num);
   Serial.print(" = ");
   for (uint8_t i = 0; i < inBytes.size(); ++i)
   {
      Serial.print(inBytes[i], DEC);
      Serial.print("(0x");
      Serial.print(inBytes[i], HEX);
      Serial.print(") ");
   }
   Serial.println("");
}

void writeCommand(uint8_t i2c_address, uint8_t reg_num, uint8_t reg_val)
{
   bool success = writeRegister(i2c_address, reg_num, reg_val);
   Serial.print(success ? "SUCCESS - 0x" : "FAILED - 0x");
   Serial.print(i2c_address, HEX);
   Serial.print(" : write ");
   Serial.print(reg_num);
   Serial.print(" = ");
   Serial.print(reg_val);
   Serial.print("(0x");
   Serial.print(reg_val, HEX);
   Serial.println(")");
}

//"W device_address reg_num reg_val - writes 'reg_val' to register 'reg_num'\n"
void writeDecCommand()
{
   char *arg;

   arg                 = sCmd.next();
   uint8_t i2c_address = atoi(arg);
   arg                 = sCmd.next();
   uint8_t reg_num     = atoi(arg);
   arg                 = sCmd.next();
   uint8_t reg_val     = atoi(arg);
   if (arg != NULL)
   {
      writeCommand(i2c_address, reg_num, reg_val);
   }
   else
   {
      Serial.println(
          "Three arguments are required : W device_address reg_num reg_val");
   }
}
//"W device_address reg_num reg_val - writes 'reg_val' to register 'reg_num'\n"
void writeHexCommand()
{
   char *arg;

   arg                 = sCmd.next();
   uint8_t i2c_address = strtoul(arg, NULL, 16);
   arg                 = sCmd.next();
   uint8_t reg_num     = strtoul(arg, NULL, 16);
   arg                 = sCmd.next();
   uint8_t reg_val     = strtoul(arg, NULL, 16);
   if (arg != NULL)
   {
      writeCommand(i2c_address, reg_num, reg_val);
   }
   else
   {
      Serial.println(
          "Three arguments are required : W device_address reg_num reg_val");
   }
}

void scanCommand()
{
   uint8_t ack, address;
   uint8_t nDevices = 0;
  
   Serial.println("Scanning...");

   for (uint8_t address = 0; address < 127; address++)
   {
      Vector<uint8_t> inBytes;
      ack = readRegister(address, 0x00, 1, inBytes);
      if (ack)
      {
         Serial.print("\n");
         Serial.print(address, DEC);
         Serial.print("(0x");
         if (address < 16) Serial.print("0");
         Serial.print(address, HEX);
         Serial.print(")");
         nDevices++;
      }
      else
      {
        Serial.print(".");
      }
   }
   if (nDevices == 0)
      Serial.println("\nNo I2C devices found");
   else
      Serial.println("\nI2C scan complete");
}

//"P sda_pin scl_pin - sets I2C pins\n"
void setI2CPins()
{
   char *arg;

   arg     = sCmd.next();
   uint8_t sda_pin = atoi(arg);
   arg     = sCmd.next();
   uint8_t scl_pin = atoi(arg);
   if (arg != NULL)
   {
      Serial.print("sda = ");
      Serial.print(sda_pin);
      Serial.print(", scl = ");
      Serial.println(scl_pin);
      
      softwarei2c.init(sda_pin, scl_pin);
      softwarei2c.begin();
      scanCommand();
   }
   else
   {
      Serial.println("Two arguments are required : P sda_pin scl_pin");
   }
}

//"R device_address reg_num reg_len - reads 'reg_len' bytes from register
//'reg_num'\n"
void readDecCommand()
{
   char *arg;

   arg                 = sCmd.next();
   uint8_t i2c_address = atoi(arg);
   arg                 = sCmd.next();
   uint8_t reg_num     = atoi(arg);
   arg                 = sCmd.next();
   uint8_t reg_len     = atoi(arg);

   if (arg != NULL)
   {
      readCommand(i2c_address, reg_num, reg_len);
   }
   else
   {
      Serial.println(
          "Three arguments are required : R device_address reg_num reg_len");
   }
}

//"R device_address reg_num reg_len - reads 'reg_len' bytes from register
//'reg_num'\n"
void readHexCommand()
{
   char *arg;

   arg                 = sCmd.next();
   uint8_t i2c_address = strtoul(arg, NULL, 16);
   arg                 = sCmd.next();
   uint8_t reg_num     = strtoul(arg, NULL, 16);
   arg                 = sCmd.next();
   uint8_t reg_len     = strtoul(arg, NULL, 16);

   if (arg != NULL)
   {
      readCommand(i2c_address, reg_num, reg_len);
   }
   else
   {
      Serial.println(
          "Three arguments are required : R device_address reg_num reg_len");
   }
}
