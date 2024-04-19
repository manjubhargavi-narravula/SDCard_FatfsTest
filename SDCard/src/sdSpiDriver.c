//**************************************************************************************************
// Medtronic - Navigation, Inc.
// 200 Beaver Brook Rd, 
// Boxborough, MA 01719 
//
// Proprietary software.
// Do not reproduce without permission in writing.
// Copyright © 2023 Medtronic - Navigation, Inc.
// All rights reserved.
//
// FILENAME:        sdSpiDriver.c
//
// SUMMARY:
// This module contains SD card SPI driver initializations and APIs.
//
// REVISION HISTORY:
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Author              Date          Comments on this revision
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Manju. N            SEP/22/2023   Initial release
//
//**************************************************************************************************

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <xc.h>
#include "sdSpiDriver.h"

#define MAX_WAIT_COUNT      10000

void (*spi2_interruptHandler)(void); 

void spi2_close(void)
{
    SPI2STATbits.SPIEN = 0;
}

//con1 == SPIxCON1, con2 == SPIxCON2, stat == SPIxSTAT, operation == Master/Slave
typedef struct { u16 con1; u16 con2; u16 stat; u08 operation;} spi2_configuration_t;
static const spi2_configuration_t spi2_configuration[] = {   
    { 0x007F, 0x0000, 0x0000, 0 }, //SDFAST_CONFIG
    { 0x0076, 0x0000, 0x0000, 0 }, //SDSLOW_CONFIG
    { 0x0122, 0x0000, 0x0000, 0 } //SPI2_DEFAULT
};

void Init_SPI2_FastSpeed(void)
{
	// setup the SPI peripheral
	SPI2STAT = 0x0;  // disable the SPI module (just in case)
	
    SPI2CON1 = 0x012F;//0x0122;	// FRAMEN = 0, SPIFSD = 0, DISSDO = 0, MODE16 = 0; SMP = 0; CKP = 1; CKE = 1; SSEN = 0; MSTEN = 1; SPRE = 0b000, PPRE = 0b01
    SPI2CON2 = 0;
    
	SPI2STAT = 0x8000; // enable the SPI module
}

void Init_SPI2_LowSpeed(void)
{
	// setup the SPI peripheral
	SPI2STAT = 0x0;  // disable the SPI module (just in case)
	
    SPI2CON1 = 0x0122;//0x0122;	// FRAMEN = 0, SPIFSD = 0, DISSDO = 0, MODE16 = 0; SMP = 0; CKP = 1; CKE = 1; SSEN = 0; MSTEN = 1; SPRE = 0b000, PPRE = 0b01
    SPI2CON2 = 0;
    
	SPI2STAT = 0x8000; // enable the SPI module
}

void Init_SPI2_Default(void)
{
	// setup the SPI peripheral
	SPI2STAT = 0x0;  // disable the SPI module (just in case)
	
    SPI2CON1 = 0x0076;//0x0122;	// FRAMEN = 0, SPIFSD = 0, DISSDO = 0, MODE16 = 0; SMP = 0; CKP = 1; CKE = 1; SSEN = 0; MSTEN = 1; SPRE = 0b000, PPRE = 0b01
    SPI2CON2 = 0;
    
	SPI2STAT = 0x8000; // enable the SPI module
}

bool spi2_open(spi_modes spiUniqueConfiguration)
{    
    switch(spiUniqueConfiguration)
    {
        case SDFAST_CONFIG:
                Init_SPI2_FastSpeed();
                SPI2STATbits.SPIEN = 1;    
                return true;        
            break;

        case SDSLOW_CONFIG:
                Init_SPI2_LowSpeed();
                SPI2STATbits.SPIEN = 1;    
                return true;
            break;
        default:
        case SPI_DEFAULT:
                Init_SPI2_Default();
                SPI2STATbits.SPIEN = 1;    
                return true;
            break; 
    }
}

u16 spi2_readByte(void)
{
    return SPI2BUF;
}

// Full Duplex SPI Functions
u08 spi2_exchangeByte(u08 data)
{
    SPI2BUF = data;
    while(!SPI2STATbits.SPIRBF);
    return SPI2BUF;
}

void spi2_exchangeBlock(void *block, size_t blockSize)
{
    u08 *data = block;
    while(blockSize--)
    {
        *data = spi2_exchangeByte(*data );
        data++;
    }
}

// Half Duplex SPI Functions
void spi2_writeBlock(void *block, size_t blockSize)
{
    u08 *data = block;
    while(blockSize--)
    {
        spi2_exchangeByte(*data++);
    }
}

void spi2_readBlock(void *block, size_t blockSize)
{
    u08 *data = block;
    while(blockSize--)
    {
        *data++ = spi2_exchangeByte(0);
    }
}

void spi2_writeByte(u16 byte)
{
    //SPI2BUF = byte;
    
    u16 waitCounter = 0;

    // Wait for any pending transmissions
    while (SPI2STATbits.SPITBF && (++waitCounter < MAX_WAIT_COUNT));

    // Word write
    if (SPI2CON1bits.MODE16)
    {
        SPI2BUF = byte;
    }
    // Byte write
    else
    {
        SPI2BUF = byte & 0xff;
    }   
}

void spi2_setSpiISR(void(*handler)(void))
{
    spi2_interruptHandler = handler;
}

void spi2_isr(void)
{
    if(IFS2bits.SPI2IF == 1){
        if(spi2_interruptHandler){
            spi2_interruptHandler();
        }
        IFS2bits.SPI2IF = 0;
    }
}

