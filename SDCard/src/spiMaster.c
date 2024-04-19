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
// FILENAME:        spiMaster.c
//
// SUMMARY:
// This module provides functions for SPI application to access SPI driver for 2 SPIs.
//
// REVISION HISTORY:
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Author              Date          Comments on this revision
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Manju N            AUG/10/2023   Initial release
//**************************************************************************************************

//**************************************************************************************************
// Includes
//**************************************************************************************************
#include "spiMaster.h"
#include "sdSpiDriver.h"

bool SDFAST_open(void);
bool SDSLOW_open(void);

const spi_master_functions_t spiMaster[] = { 
    {spi2_close, SDFAST_open, spi2_exchangeByte, spi2_exchangeBlock, spi2_writeBlock, spi2_readBlock, spi2_writeByte, spi2_readByte, spi2_setSpiISR, spi2_isr },
    {spi2_close, SDSLOW_open, spi2_exchangeByte, spi2_exchangeBlock, spi2_writeBlock, spi2_readBlock, spi2_writeByte, spi2_readByte, spi2_setSpiISR, spi2_isr }
};

bool SDFAST_open(void){
    return spi2_open(SDFAST_CONFIG);    
}

bool SDSLOW_open(void){
    return spi2_open(SDSLOW_CONFIG);
}

/**
 *  \ingroup doc_driver_spi_code
 *  \brief Open the SPI interface.
 *
 *  This function is to keep the backward compatibility with older API users
 *  \param[in] configuration The configuration to use in the transfer
 *
 *  \return Initialization status.
 *  \retval false The SPI open was unsuccessful
 *  \retval true  The SPI open was successful
 */
bool spi_master_open(spi_master_configurations_t config){
    switch(config){
        case SDFAST:
            return SDFAST_open();
        case SDSLOW:
            return SDSLOW_open();
        default:
            return 0;
    }
}

/**
 End of File
 */
