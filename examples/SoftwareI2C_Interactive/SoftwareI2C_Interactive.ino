#include <SoftwareI2C.h>  // https://github.com/micooke/SoftwareI2C

SoftwareI2C softwarei2c;

#include "SerialCommands.h"
/*
Note : Serial Commands requires the following libraries
#include <SerialCommand.h> // https://github.com/kroimon/Arduino-SerialCommand
#include <Vector.h> // https://github.com/zacsketches/Arduino_Vector
*/

void setup(void)
{
   Serial.begin(9600);
   Serial.println("SDA,SCL = 0,1");

   sCmd.addCommand("P", setI2CPins);
   sCmd.addCommand("R", readDecCommand);
   sCmd.addCommand("RX", readHexCommand);
   sCmd.addCommand("W", writeDecCommand);
   sCmd.addCommand("WX", writeHexCommand);
   sCmd.addCommand("S", scanCommand);
   sCmd.setDefaultHandler(unrecognized);

   softwarei2c.begin();  // defaults to (sda,scl) = (0,1)

   unrecognized(NULL);
}

void loop(void) { sCmd.readSerial(); }
