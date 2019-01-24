/*
  SoftwareI2C.cpp
  2012 Copyright (c) Seeed Technology Inc.  All right reserved.

  Author:Loovee
  Author:Loovee

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

#include <Arduino.h>

#include "SoftwareI2C.h"

void SoftwareI2C::setClock(uint32_t frequency_Hz)
{
    if (frequency_Hz == 0)
    {
        _bit_delay_us = 0;
        _half_bit_delay_us = 0;
        return;
    }
    else if (frequency_Hz <= 50000)
    {
      frequency_Hz = 500000;
    }
    else if (frequency_Hz <= 100000)
    {
      frequency_Hz = 100000;
    }
    else if (frequency_Hz <= 250000)
    {
      frequency_Hz = 250000;
    }
    else
    {
      frequency_Hz = 400000;
    }


    // Precalculate the bit_delay, in number of 4-cycle delays
    _bit_delay_us = (1000000 / frequency_Hz);
    _half_bit_delay_us = _bit_delay_us/2;
 }

/*************************************************************************************************
 * Function Name: begin
 * Description:  config IO
 * Parameters: Sda: Scl:
 * Return: None
*************************************************************************************************/
void SoftwareI2C::begin()
{
    pinMode(_pinScl, OUTPUT);
    pinMode(_pinSda, OUTPUT);
    digitalWrite(_pinScl, HIGH);
    digitalWrite(_pinSda, HIGH);
}
/*************************************************************************************************
 * Function Name: sdaGet
 * Description:  get sda digital value
 * Parameters: None
 * Return: HIGH or LOW
*************************************************************************************************/
uint8_t SoftwareI2C::sdaGet()
{
    pinMode(_pinSda, INPUT);
    uint8_t value = digitalRead(_pinSda);
    pinMode(_pinSda, OUTPUT);
    return value;
}

/*************************************************************************************************
 * Function Name: sdaSet
 * Description:  set sda digital value
 * Parameters: ucDta: HIGH or LOW
 * Return: None
*************************************************************************************************/
void SoftwareI2C::sdaSet(uint8_t ucDta)
{
    digitalWrite(_pinSda, ucDta);
}

/*************************************************************************************************
 * Function Name: sclSet
 * Description:  set scl digital value
 * Parameters: ucDta: HIGH or LOW
 * Return: None
*************************************************************************************************/
void SoftwareI2C::sclSet(uint8_t ucDta)
{
    digitalWrite(_pinScl, ucDta);
}

/*************************************************************************************************
 * Function Name: getAck
 * Description:  get ack
 * Parameters: None
 * Return: 0 – Nak; 1 – Ack
*************************************************************************************************/
uint8_t SoftwareI2C::getAck(void)
{
    sdaSet(HIGH);
    sclSet(HIGH);
    tunedDelay(_half_bit_delay_us); // sample in the middle of the pulse
    uint8_t Ack = sdaGet(); // get ack
    sdaSet(LOW);
    sclSet(LOW);
    
    return Ack;
}

/*************************************************************************************************
 * Function Name: sendStart
 * Description:   send start clock
 * Parameters: None
 * Return: None
*************************************************************************************************/
void SoftwareI2C::sendStart(void)
{
    uint8_t sdaValue = sdaGet();
    if (sdaValue == 0)
    {
        sdaSet(HIGH);
        tunedDelay(_half_bit_delay_us);
        sclSet(HIGH);
        tunedDelay(_bit_delay_us);
    }
    sdaSet(LOW);
    tunedDelay(_half_bit_delay_us);
    sclSet(LOW);
    tunedDelay(_bit_delay_us);
}

/*************************************************************************************************
 * Function Name: sendStop
 * Description:  send stop clock
 * Parameters: None
 * Return: None
*************************************************************************************************/
void SoftwareI2C::sendStop(void)
{
    if (sdaGet() == HIGH)
    {
        sdaSet(LOW);
        tunedDelay(_bit_delay_us);
    }
    sclSet(HIGH);
    tunedDelay(_bit_delay_us);
    sdaSet(HIGH);
    tunedDelay(_bit_delay_us);
}

/*************************************************************************************************
 * Function Name: sendByte
 * Description:  send a byte
 * Parameters: ucDta: data to send
 * Return: None
*************************************************************************************************/
void SoftwareI2C::sendByte(uint8_t ucDta)
{
    uint32_t timer_t = micros();
    for(uint8_t i=0; i<8; ++i)
    {
        timer_t = micros();
        sdaSet((ucDta&0x80)!=0);
        tunedDelay(_bit_delay_us);
        sclSet(HIGH);
        tunedDelay(_bit_delay_us);
        sclSet(LOW);
        tunedDelay(_bit_delay_us);
        ucDta <<= 1;
    }
}

/*************************************************************************************************
 * Function Name: sendByteAck
 * Description:  send a byte and get ack signal
 * Parameters: ucDta: data to send
 * Return: 0: get nak  1: get ack
*************************************************************************************************/
uint8_t SoftwareI2C::sendByteAck(uint8_t ucDta)
{
    sendByte(ucDta);
    return getAck();
}

/*************************************************************************************************
 * Function Name: beginTransmission
 * Description:  send begin signal
 * Parameters: divider – clock divider
 * Return: None
*************************************************************************************************/
void SoftwareI2C::beginTransmission(uint8_t addr)
{
    _i2c_address = addr;
    _transmissionBegun = true;
    sendStart();                  // start signal
    //sendByteAck(_i2c_address<<1); // send write address and get ack
    write(_i2c_address<<1);
}

/*************************************************************************************************
 * Function Name: endTransmission
 * Description:  triggers sending the write commands and sends the stop signal
 * Parameters: None
 * Return: 0: get ack  1: get nak
*************************************************************************************************/
uint8_t SoftwareI2C::endTransmission(bool stopBit)
{
    uint8_t _success = 0;
    if (_transmissionBegun == true)
    {
        while (txBuffer.available())
        {
            _success = sendByteAck(txBuffer.read_char());
        }

        sendStop();
        _transmissionBegun = false;
    }
    
    return _success;
}

/*************************************************************************************************
 * Function Name: write
 * Description:  send a byte
 * Parameters: dta: data to send
 * Return: 0: get nak  1: get ack
*************************************************************************************************/  
uint8_t SoftwareI2C::write(uint8_t dta)
{
    //return sendByteAck(dta);
    // No writing, without begun transmission or a full buffer
    if ( !_transmissionBegun || txBuffer.isFull() )
    {
        return 0 ;
    }

    txBuffer.store_char( dta ) ;
    return 1 ;
}

/*************************************************************************************************
 * Function Name: write
 * Description:  write array
 * Parameters: len - length of the array
               *dta - array to be sent
 * Return: 0: get nak  1: get ack
*************************************************************************************************/
uint8_t SoftwareI2C::write(uint8_t *dta, uint8_t len)
{
    /*uint8_t ack = 1;
    for(uint16_t i=0; i<len; ++i)
    {
        if(write(dta[i]) == 0)
        {
            ack = 0;
            break;
        }
    }

    return ack;
    */
  ///Try to store all data
  for(size_t i = 0; i < len; ++i)
  {
    //Return the number of data stored, when the buffer is full (if write return 0)
    if(!write(dta[i]))
    {
      return i;
    }
  }

  //All data stored
  return len;
}

/*************************************************************************************************
 * Function Name: requestFrom
 * Description:  request data from slave
 * Parameters: addr - address of slave
               len  - length of request
 * Return: 0: get nak  1: get ack
*************************************************************************************************/
uint8_t SoftwareI2C::requestFrom(uint8_t addr, uint8_t len)
{
    sendStart();                             // start signal
    _i2c_address = addr;
    //_rx_len = len;

    uint8_t ret = sendByteAck((_i2c_address<<1)+1);  // send read address and get ack
    //sclSet(LOW);
    _transmissionBegun = true;

    size_t byteRead = 0;

    for (byteRead = 0; byteRead < len; byteRead++)
    {
        uint8_t ucRt = 0;
        // get the upper 7 bits of the byte
        for(uint8_t b=0; b<7; ++b)
        {
            sclSet(HIGH);
            tunedDelay(_half_bit_delay_us);
            ucRt = (ucRt << 1) + sdaGet();
            sclSet(LOW);
            tunedDelay(_half_bit_delay_us);
        }

        // get the last bit and switch to output prior to setting scl low
        // (sda toggles if you switch pinMode after scl is set low)
        sclSet(HIGH);
        tunedDelay(_half_bit_delay_us);
        ucRt = (ucRt << 1) + sdaGet();
        sclSet(LOW);
        tunedDelay(_half_bit_delay_us);

        // if last packet send NACK, otherwise send ACK
        uint8_t last_packet = (byteRead +1 == len);//(--_rx_len == 0);

        sdaSet(last_packet);
        sclSet(HIGH);
        tunedDelay(_half_bit_delay_us);
        sclSet(LOW);
        sdaSet(LOW);
        tunedDelay(_half_bit_delay_us);

        // store the byte
        rxBuffer.store_char(ucRt);
    }

    // validate the return using the sendByteAck
    return byteRead * (!ret);//ret;
}

/*************************************************************************************************
 * Function Name: read
 * Description:  read a byte from i2c
 * Parameters: None
 * Return: data get
*************************************************************************************************/
uint8_t SoftwareI2C::read()
{
    return rxBuffer.read_char();
    /*
    if(_rx_len == 0) return 0;

    uint8_t ucRt = 0;

    for(uint8_t i=0; i<7; ++i)
    {
        sclSet(HIGH);
        tunedDelay(_half_bit_delay_us);
        ucRt = (ucRt << 1) + sdaGet();
        sclSet(LOW);
        tunedDelay(_half_bit_delay_us);
    }

    // get the last bit and switch to output prior to setting scl low
    // (sda toggles if you switch pinMode after scl is set low)
    sclSet(HIGH);
    tunedDelay(_half_bit_delay_us);
    ucRt = (ucRt << 1) + sdaGet();
    sclSet(LOW);
    tunedDelay(_half_bit_delay_us);

    // if last packet send NACK, otherwise send ACK
    uint8_t last_packet = (--_rx_len == 0);

    sdaSet(last_packet);
    sclSet(HIGH);
    tunedDelay(_half_bit_delay_us);
    sclSet(LOW);
    sdaSet(LOW);
    tunedDelay(_half_bit_delay_us);
    
    return ucRt;
    */
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/