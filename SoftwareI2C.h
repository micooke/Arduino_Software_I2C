/*
  SoftwareI2C.h
  2012 Copyright (c) Seeed Technology Inc.  All right reserved.

  This is a Software I2C Library, can act as I2c master mode.
  
  @author  Loovee
  @version 1.0
  @date    01-Nov-2013

  @author  Mark Cooke
  @version 1.1
  @date    01-Sep-2017
  @brief   uchar -> uint8_t, standardise interface to better match Wire
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef _SOFTWAREI2C_H_
#define _SOFTWAREI2C_H_

#include <stdint.h>

#define  GETACK        1 // get ack                        
#define  GETNAK        0 // get nak   
   
class SoftwareI2C
{
private:
    uint8_t _pinSda;
    uint8_t _pinScl;
    
    uint8_t _recv_len;
    uint8_t _sda_in_out;
    
    inline void sdaSet(uint8_t ucDta); 
    inline void sclSet(uint8_t ucDta);                                                                   

    inline void sendStart(void);
    inline void sendStop(void);
    inline uint8_t getAck(void);
    inline void sendByte(uint8_t ucDta);
    inline uint8_t sendByteAck(uint8_t ucDta);  // send byte and get ack
    
public:
    SoftwareI2C() : _pinScl(0), _pinSda(1), _recv_len(0), _sda_in_out(0) {}
    void begin(uint8_t pinSda = 0, uint8_t pinScl = 1); 
    uint8_t beginTransmission(uint8_t addr);
    uint8_t endTransmission(bool stopBit = false);
    void end() {}
    
    uint8_t write(uint8_t dta);
    uint8_t write(uint8_t *dta, uint8_t len);
    uint8_t requestFrom(uint8_t addr, uint8_t len);
    uint8_t read();
    uint8_t available() { return _recv_len; }
};

#endif
/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
