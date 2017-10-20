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

void SoftwareI2C::setClock(uint32_t baudrate)
{
    uint32_t frequency;

    if (!baudrate)
    {
        _bit_delay = 0;
        _bit_delay_us = 0;
        return;
    }
    else if (baudrate <= 100000)
    {
      frequency = 100000;
    }
    else if (baudrate <= 250000)
    {
      frequency = 250000;
    }
    else
    {
      frequency = 400000;
    }


    // Precalculate the bit_delay, in number of 4-cycle delays
    _bit_delay = (F_CPU / frequency) / 4;
    _bit_delay_us = (4000000 / F_CPU) * _bit_delay;
 }

/*************************************************************************************************
 * Function Name: begin
 * Description:  config IO
 * Parameters: Sda: Scl:
 * Return: none
*************************************************************************************************/
void SoftwareI2C::begin()
{
    pinMode(_pinScl, OUTPUT);
    pinMode(_pinSda, OUTPUT);
    digitalWrite(_pinScl, HIGH);
    digitalWrite(_pinSda, HIGH);
}

/*************************************************************************************************
 * Function Name: sdaSet
 * Description:  set sda
 * Parameters: ucDta: HIGH or LOW
 * Return: none
*************************************************************************************************/
void SoftwareI2C::sdaSet(uint8_t ucDta)
{
    digitalWrite(_pinSda, ucDta);
    digitalWrite(_pinSda, ucDta);
}

/*************************************************************************************************
 * Function Name: sclSet
 * Description:  set scl
 * Parameters: ucDta: HIGH or LOW
 * Return: none
*************************************************************************************************/
void SoftwareI2C::sclSet(uint8_t ucDta)
{
    digitalWrite(_pinScl, ucDta);
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
    uint8_t ack = 1;
    pinMode(_pinSda, INPUT);

    sclSet(HIGH);
    if(digitalRead(_pinSda) == 0) // get ack
    {
        pinMode(_pinSda, OUTPUT);
        sdaSet(LOW);
        sclSet(LOW);
        return 1;
    }
    else
    {
        uint32_t timer_t = micros();
        while(micros() - timer_t < (_bit_delay_us / 2))
        {
            if(digitalRead(_pinSda) == 0) // get ack
            {
                pinMode(_pinSda, OUTPUT);
                sdaSet(LOW);
                sclSet(LOW);
                return 1;
            }
        }
    }
    pinMode(_pinSda, OUTPUT);
    sdaSet(LOW);
    sclSet(LOW);
    return 0;
}

/*************************************************************************************************
 * Function Name: sendStart
 * Description:   send start clock
 * Parameters: None
 * Return: None
*************************************************************************************************/
void SoftwareI2C::sendStart(void)
{
    sdaSet(LOW);
    sclSet(LOW);
}

/*************************************************************************************************
 * Function Name: setClockDivider
 * Description:  setup clock divider for spi bus
 * Parameters: divider – clock divider
 * Return: 0 – setup ok; 1 – setup failed
*************************************************************************************************/
void SoftwareI2C::sendStop(void)
{
    sclSet(HIGH);
    sdaSet(HIGH);
}

/*************************************************************************************************
 * Function Name: sendByte
 * Description:  send a byte
 * Parameters: ucDta: data to send
 * Return: None
*************************************************************************************************/
void SoftwareI2C::sendByte(uint8_t ucDta)
{
    for(uint8_t i=0; i<8; ++i)
    {
        sdaSet((ucDta&0x80)!=0);
        //ucDta <<= 0;
        sclSet(HIGH);
        //sdaSet((ucDta&0x80)!=0); // for timing
        sclSet(LOW);
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
    uint8_t ack = 0;
    sendByte(ucDta);
    _success = getAck();
    return _success;
}

/*************************************************************************************************
 * Function Name: beginTransmission
 * Description:  send begin signal
 * Parameters: divider – clock divider
 * Return: 0: get nak  1: get ack
*************************************************************************************************/
void SoftwareI2C::beginTransmission(uint8_t addr)
{
    _i2c_address = addr;
    sendStart();                  // start signal
	sendByteAck(_i2c_address<<1); // send write address and get ack
    _transmissionBegun = true;
}

/*************************************************************************************************
 * Function Name: endTransmission
 * Description:  send stop signal
 * Parameters: None
 * Return: 0: get ack  1: get nak
*************************************************************************************************/
uint8_t SoftwareI2C::endTransmission(bool stopBit)
{
    
    if (_transmissionBegun == false)
    {
        _success = 0;
    }
    else
    {
        sendStop();    
    }

    _transmissionBegun = false;
    return !_success;
}

/*************************************************************************************************
 * Function Name: write
 * Description:  send a byte
 * Parameters: dta: data to send
 * Return: 0: get nak  1: get ack
*************************************************************************************************/  
uint8_t SoftwareI2C::write(uint8_t dta)
{
    return sendByteAck(dta);
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
    uint8_t ack = 1;
    for(uint16_t i=0; i<len; ++i)
    {
        if(write(dta[i]) == 0)
        {
            ack = 0;
            break;
        }
    }

    return ack;
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
    _rx_len = len;
    uint8_t ret = sendByteAck((_i2c_address<<1)+1);  // send read address and get ack
    //sclSet(LOW);
    _transmissionBegun = true;
    return ret;
}

/*************************************************************************************************
 * Function Name: read
 * Description:  read a byte from i2c
 * Parameters: None
 * Return: data get
*************************************************************************************************/
uint8_t SoftwareI2C::read()
{
    if(_rx_len == 0) return 0;

    uint8_t ucRt = 0;

    pinMode(_pinSda, INPUT);
    for(uint8_t i=0; i<7; ++i)
    {
        sclSet(HIGH);
        ucRt = (ucRt << 1) + digitalRead(_pinSda);
        sclSet(LOW);
    }

    // get the last bit and switch to output prior to setting scl low
    // (sda toggles if you switch pinMode after scl is set low)
    sclSet(HIGH);
    ucRt = (ucRt << 1) + digitalRead(_pinSda);
    pinMode(_pinSda, OUTPUT);
    sclSet(LOW);    

    if(--_rx_len>0)     // send ACK
    {
        sdaSet(LOW);
        sclSet(HIGH);
        sclSet(LOW);
        sdaSet(HIGH);
    }
    else               // send NAK
    {
        sdaSet(HIGH);
        sclSet(HIGH);
        sclSet(LOW);
        sdaSet(LOW);
        sendStop();
    }

    return ucRt;
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/