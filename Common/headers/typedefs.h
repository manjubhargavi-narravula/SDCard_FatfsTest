//**************************************************************************************************
// Medtronic - Navigation, Inc.
// 300 Foster St.
// Littleton, MA 01460
//
// Proprietary software.
// Do not reproduce without permission in writing.
// Copyright © 2014 Medtronic - Navigation, Inc.
// All rights reserved.
//
// SUMMARY:
// This header contains common definitions for the O2 firmware.
//
// FILENAME:        typedefs.h
//
// REVISION HISTORY:
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Author              Date          Comments on this revision
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// M. Marrama          JUL/31/2013   Initial release
// M. Marrama          DEC/04/2013   Add support for Windows OS
// M. Marrama          APR/15/2014   Add signed types for Windows OS
// M. Marrama          SEP/29/2014   TTP 1602

//**************************************************************************************************

#ifndef _TYPEDEFS_H_
#define _TYPEDEFS_H_

//**************************************************************************************************
// Includes
//**************************************************************************************************

#ifndef WINXL
#include <stdbool.h> // bool, true, false
#endif
#include <stddef.h>  // NULL
#include <stdint.h>  // integer mininums and maximums
#include "typedefs.h"
//**************************************************************************************************
// Definitions
//**************************************************************************************************

#ifdef WINXL
// Basic value types
typedef unsigned char   u08;
typedef char            i08;
typedef unsigned short  u16;
typedef short           i16;
typedef unsigned long   u32;
typedef long            i32;

#define bool u08

#ifndef false
#define false 0
#endif

#ifndef true
#define true 1
#endif
#else
// Process exit status
#define	EXIT_SUCCESS            0
#define	EXIT_FAILURE            1

// Basic integer types
typedef char                    i08;
typedef unsigned char           u08;
typedef int                     i16;
typedef unsigned int            u16;
typedef long int                i32;
typedef unsigned long int       u32;
typedef long long int           i64;
typedef unsigned long long int  u64;

// Basic float types
typedef float                   f32;
typedef long double             f64;

// Interrupt Priority Levels
#define IP_HIGHEST              7U
#define IP_HIGHER               6U
#define IP_HIGH                 5U
#define IP_MEDIUM               4U
#define IP_LOW                  3U
#define IP_LOWER                2U
#define IP_LOWEST               1U
#define IP_IDLE                 0U

// Signal State
#define LO 0U
#define HI 1U

// Switch State
#ifndef OFF
#define OFF 0U
#endif
#ifndef ON
#define ON  1U
#endif

// Function Status
#define FAILURE 0U
#define SUCCESS 1U

// PIC I/O Pin Direction
#define PIN_OUT 0U
#define PIN_IN  1U
#endif

// Application run mode
typedef unsigned char RUN_MODE;
#define RUN_APPLICATION         0U
#define RESET_APPLICATION       1U
#define EXIT_APPLICATION        2U
#define RESET_CAN_COMM          3U

#endif
