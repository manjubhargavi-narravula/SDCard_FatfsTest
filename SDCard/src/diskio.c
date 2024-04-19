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
// FILENAME:        diskio.c
//
// SUMMARY:
// This module provides functions for Storage control modules to the FatFs module with a defined API
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
#include <stdbool.h>
#include "diskio.h"		/* FatFs lower layer API */
#include "sdSpi.h"
#include <xc.h>

/* Definitions of physical drive number for each drive */
enum DRIVER_LIST{
    DRVA = 0,
};

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
    BYTE pdrv    /* Physical drive number to identify the drive */
)
{
    DSTATUS stat = STA_NOINIT;

    switch (pdrv) {

        case DRVA:
            if ( SD_SPI_IsMediaPresent() == true) //false)  - changed by Manju
            {
               stat = STA_NODISK;
            }
            else if ( SD_SPI_IsMediaInitialized() == true)
            {
                stat &= ~STA_NOINIT;
            }
        
            if ( SD_SPI_IsWriteProtected() == true)
            {
                stat |= STA_PROTECT;
            }

            break;

        default:
            break;
    }
    return stat;
}



/*-----------------------------------------------------------------------*/
/* Initialize a Drive                                                    */
/*-----------------------------------------------------------------------*/
bool sdspists;
DSTATUS disk_initialize (
    BYTE pdrv    /* Physical drive number to identify the drive */
)
{
    DSTATUS stat = STA_NOINIT;

    sdspists = SD_SPI_MediaInitialize();
    switch (pdrv) {
        case DRVA :
            //if(SD_SPI_MediaInitialize() == true)
            if(sdspists == true)
            {
                stat = RES_OK;
            }
            else
            {
                stat = RES_ERROR;
            }
            break;
        default:
            break;
    }

    return stat;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
    BYTE pdrv,    /* Physical drive number to identify the drive */
    BYTE *buff,   /* Data buffer to store read data */
    DWORD sector, /* Start sector in LBA */
    UINT count    /* Number of sectors to read */
)
{
    DRESULT res = RES_PARERR;

    switch (pdrv) {
        case DRVA :
            if(SD_SPI_SectorRead(sector, buff, count) == true)
            {
                res = RES_OK;
            }
            else
            {
                res = RES_ERROR;
            }
            break;

        default:
            break;
    }

    return res;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write (
    BYTE pdrv,          /* Physical drive number to identify the drive */
    const BYTE *buff,   /* Data to be written */
    DWORD sector,       /* Start sector in LBA */
    UINT count          /* Number of sectors to write */
)
{
    DRESULT res = RES_PARERR;

    switch (pdrv) {
        case DRVA :
            if(SD_SPI_SectorWrite(sector, buff, count) == true)
            {
                res = RES_OK;
            }
            else
            {
                res = RES_ERROR;
            }
            break;

        default:
            break;
    }

    return res;
}



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
    BYTE pdrv,    /* Physical drive number (0..) */
    BYTE cmd,     /* Control code */
    void *buff    /* Buffer to send/receive control data */
)
{
    DRESULT res = RES_OK;

    switch (pdrv) {
        case DRVA :
            return res;

        default:
            break;
    }

    return RES_PARERR;
}

