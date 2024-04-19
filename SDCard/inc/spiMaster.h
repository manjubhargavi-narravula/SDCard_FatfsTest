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
// FILENAME:        spiMaster.h
//
// SUMMARY:
// This header serves as the interface for spiMaster.c.
//
// REVISION HISTORY:
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Author              Date          Comments on this revision
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Manju N          SEP/25/2023         Initial release
//
//**************************************************************************************************


#ifndef SPI_MASTER_H
#define SPI_MASTER_H

/**
  Section: Included Files
 */
#include <stdint.h>
#include <stdbool.h>
#include "sdSpiDriver.h"
#include "typedefs.h"
/**
*   \ingroup doc_driver_spi_code
*   \enum spi_master_configurations_t spi_master.h
*/
typedef enum { 
    SDFAST,
    SDSLOW
} spi_master_configurations_t;

/**
*   \ingroup doc_driver_spi_code
*   \struct spi_master_functions_t spi_master.h
*/
typedef struct {    void (*spiClose)(void);
                    bool (*spiOpen)(void);
                    u08 (*exchangeByte)(u08 b);
                    void (*exchangeBlock)(void * block, size_t blockSize);
                    void (*writeBlock)(void * block, size_t blockSize);
                    void (*readBlock)(void * block, size_t blockSize);
                    void (*writeByte)(u16 byte);
                    u16 (*readByte)(void);
                    void (*setSpiISR)(void(*handler)(void));
                    void (*spiISR)(void);
} spi_master_functions_t;

extern const spi_master_functions_t spiMaster[];

bool spi_master_open(spi_master_configurations_t config);   //for backwards compatibility


#endif	// _SPI_MASTER_H