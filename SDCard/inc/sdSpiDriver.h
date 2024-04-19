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
// FILENAME:        sdSpiDriver.h
//
// SUMMARY:
// This header serves as the interface for sdSpiDriver.c.
//
// REVISION HISTORY:
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Author              Date          Comments on this revision
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Manju N          SEP/25/2023         Initial release
//
//**************************************************************************************************

#ifndef SDSPI_DRIVER_H
#define SDSPI_DRIVER_H

#include <xc.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "typedefs.h"

#define INLINE  inline 

typedef enum { 
    SDFAST_CONFIG,
    SDSLOW_CONFIG,
    SPI_DEFAULT
} spi_modes;

/* arbitration interface */

//INLINE void spi2_close(void);
void spi2_close(void);

//bool spi2_open(spi2_modes spiUniqueConfiguration);
bool spi2_open(spi_modes spiUniqueConfiguration);
/* SPI native data exchange function */
u08 spi2_exchangeByte(u08 b);
/* SPI Block move functions }(future DMA support will be here) */
void spi2_exchangeBlock(void *block, size_t blockSize);
void spi2_writeBlock(void *block, size_t blockSize);
void spi2_readBlock(void *block, size_t blockSize);

void spi2_writeByte(u16 byte);
u16 spi2_readByte(void);

void spi2_isr(void);
void spi2_setSpiISR(void(*handler)(void));

#endif // SPI2_DRIVER_H
