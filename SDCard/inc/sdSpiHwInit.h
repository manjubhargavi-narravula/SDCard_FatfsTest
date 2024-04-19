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
// FILENAME:        sdSpiHwInit.h
//
// SUMMARY:
// This header is the interface for the sdSpi.c module.  
//
// REVISION HISTORY:
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Author              Date          Comments on this revision
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Manju N            AUG/10/2023   Initial release
//**************************************************************************************************
#ifndef SDSPI_HWINIT_H
#define SDSPI_HWINIT_H
//**************************************************************************************************
// Includes
//**************************************************************************************************
#include <xc.h>

// O-arm PCB SPI1 settings

#define SDCard_CD1_SetHigh()          (_LATA12 = 1)
#define SDCard_CD1_SetLow()           (_LATA12 = 0)
#define SDCard_CD1_Toggle()           (_LATA12 ^= 1)
#define SDCard_CD1_GetValue()         _RA12
#define SDCard_CD1_SetDigitalInput()  (_TRISA12 = 1)
#define SDCard_CD1_SetDigitalOutput() (_TRISA12 = 0)

#define SCK1_SetHigh()              (_LATF6 = 1)
#define SCK1_SetLow()               (_LATF6 = 0)
#define SCK1_Toggle()               (_LATF6 ^= 1)
#define SCK1_GetValue()             _RF6
#define SCK1_SetDigitalInput()      (_TRISF6 = 1)
#define SCK1_SetDigitalOutput()     (_TRISF6 = 0)

#define SDI1_SetHigh()              (_LATF7 = 1)
#define SDI1_SetLow()               (_LATF7 = 0)
#define SDI1_Toggle()               (_LATF7 ^= 1)
#define SDI1_GetValue()             _RF7
#define SDI1_SetDigitalInput()      (_TRISF7 = 1)
#define SDI1_SetDigitalOutput()     (_TRISF7 = 0)

#define SDO1_SetHigh()              (_LATF8 = 1)
#define SDO1_SetLow()               (_LATF8 = 0)
#define SDO1_Toggle()               (_LATF8 ^= 1)
#define SDO1_GetValue()             _RF8
#define SDO1_SetDigitalInput()      (_TRISF8 = 1)
#define SDO1_SetDigitalOutput()     (_TRISF8 = 0)

#define SDCard_CS1_SetHigh()         (_LATB6 = 1)
#define SDCard_CS1_SetLow()          (_LATB6 = 0)
#define SDCard_CS1_Toggle()          (_LATB6 ^= 1)
#define SDCard_CS1_GetValue()         _RB6
#define SDCard_CS1_SetDigitalInput()  (_TRISB6 = 1)
#define SDCard_CS1_SetDigitalOutput() (_TRISB6 = 0)

#endif
