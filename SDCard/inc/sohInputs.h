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
// FILENAME:        sohInputs.h
//
// SUMMARY:
// This header serves as the interface for sohInputs.c.
//
// REVISION HISTORY:
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Author              Date          Comments on this revision
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Manju N          SEP/25/2023         Initial release
//
//**************************************************************************************************

#ifndef SOHINPUTS_H
#define	SOHINPUTS_H

#ifdef	__cplusplus
extern "C" {
#endif

//**************************************************************************************************
//  Includes
//**************************************************************************************************    
#include <stdbool.h>
#include <typedefs.h>
//#include "userControlPanelMI.h"
#include "typedefs.h"
#include "ff.h"
//#include "SOH_test_v4.h"
//**************************************************************************************************
//  Global Macro Definitions
//**************************************************************************************************

#define BAT_BKUP_FOLDER         "/BatteryLog"
#define BAT_BKUP_PATH           "0:/" BAT_BKUP_FOLDER
#define MAX_PATHNAME_LEN        128

// SOH Algorithm input data from UCP Firmware
typedef struct UCP_SOH_ALG_INPUTS_ {
    u32 BatteryAgeInSec;
    u32 BatteryAgeInYear;
    u32 RemainCapFromBMS;
    bool SOHInitFlag;
    u32 NewFullCapValue;
} UCP_SOH_ALG_INPUTS;
    
// UCP calculated variables for SOH algorithm
typedef struct UCP_SCC_CAN_DATA_
{
    u32 CurrentTemp1Value;
    u32 RemCap;
    u32 FullCap;
    u32 SoC;
    u32 SoH;
} UCP_SCC_CAN_DATA;

typedef struct UCP_RUNNING_AVG_DATA_ {
    u32 RunAvgT1Val; 
    u32 RunAvgT2Val; 
    u32 RunAvgT3Val; 
    u32 PrevT1AvgVal; 
    u32 PrevT2AvgVal; 
    u32 PrevT3AvgVal; 
    u32 LatestSDCTemp1;
    u32 RunAvgOfDCT2;
    u32 RunAvgOfDCT3;
}UCP_RUNNING_AVG_DATA;

//**************************************************************************************************
//  Global variables
//**************************************************************************************************
extern UCP_SCC_CAN_DATA UcpSccCanData;

extern UCP_SOH_ALG_INPUTS UcpSohAlgInputData;
extern UCP_RUNNING_AVG_DATA ucpRungAvgData;
extern bool _bUcpStateChanged;
//**************************************************************************************************
//  Global Function Prototypes
//**************************************************************************************************
void calcAvgTempOfUcpOffState(void);
void batteryThirtySecCalc(void);
void batteryOneSecCalc(void);
bool getSdCardMountStatus(void);
bool getSdCardChecksum(void);
void powerOnUcpSohInit(void);
void InitSohTimers(void);
bool IsSdCardExists(void);

#ifdef	__cplusplus
}
#endif

#endif	/* SOHINPUTS_H */

