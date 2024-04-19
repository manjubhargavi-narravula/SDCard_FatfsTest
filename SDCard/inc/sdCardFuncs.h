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
// FILENAME:        sdCardFuncs.h
//
// SUMMARY:
// This header serves as the interface for sdCardFuncs.c.
//
// REVISION HISTORY:
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Author              Date          Comments on this revision
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Manju N          SEP/25/2023         Initial release
//
//**************************************************************************************************

#ifndef SDCARDFUNC_H
#define SDCARDFUNC_H

//**************************************************************************************************
// Includes
//**************************************************************************************************

#include <stdbool.h>
#include <typedefs.h>

//**************************************************************************************************
// Definitions
//**************************************************************************************************

#define VALID       1
#define INVALID     0

#define FILENAME_MAXLEN     50
#define READ_ROW_LENGTH     50  // Adjust the csvFileLastRowData size according to the number of bytes you want to read

typedef enum {
    BATTERY_STATE_INVALID = 0,
    BATTERY_STATE_VALID,
    BATTERY_STATE_NEW,
    BATTERY_STATE_OVR_NIGHT_CHRG,            
    BATTERY_STATE_STANDBY,
    BATTERY_STATE_CYCLING,
    BATTERY_STATE_SHUTDOWN,
    BATTERY_STATE_ERROR,
    BATTERY_STATE_BATTERY_LEVEL,
    BATTERY_STATE_STORAGE,
    BATTERY_STATE_NOT_CHARGING,
    BATTERY_STATE_FILE_CREATE_ERROR,
    BATTERY_STATE_FILE_NAME_ERROR,
    BATTERY_STATE_CHKSUM_ERR,
    BATTERY_STATE_SEDECAL_ERR,
    BATTERY_STATE_SDCARD_MNT_ERR,
    BATTERY_STATE_FOLDER_ERR
}BATTERY_STATES;

typedef enum {
    RTC,
    TEMP1,   
    REMAINCAP,
    FULLCAP,
    SOC,
    SOH,
    MAXPARAMS        
}SCCCAN_PARAMS;

typedef enum {
    UNUSED = 0,
    SDC_TEMP1    
}SDCARD_PARAMS;

//**************************************************************************************************
// Global Variables
//**************************************************************************************************

extern i08 SedecalNewBatSerNum[FILENAME_MAXLEN];
extern i08 *NewSedBatFileName;;
extern i08 sccCanFile[FILENAME_MAXLEN], sohInputFile[FILENAME_MAXLEN];
extern i08 csvFileLastRowData[READ_ROW_LENGTH];

typedef struct UCP_DC_VALUES_ {
    u32 DutyCycleT2; 
    u32 DutyCycleT3; 
    u32 DutyCycleT2OfDay; 
    u32 DutyCycleT3OfDay;
    u32 DCT2EntryTime; 
    u32 DCT2PartOfDay; 
    u32 DCT3EntryTime; 
    u32 DCT3PartOfDay;
}UCP_SOH_DC_DATA;

//extern bool T1StateFlag, T2StateFlag, T3StateFlag;
typedef struct UCP_SOH_STATE_DATA_ {
    bool T1StateFlag;
    bool T2StateFlag;
    bool T3StateFlag;
    u32 T1SampleCnt;
    u32 T2SampleCnt; 
    u32 T3SampleCnt;
}UCP_SOH_STATE_DATA;

//**************************************************************************************************
// Global variables
//**************************************************************************************************
extern UCP_SOH_DC_DATA ucpSohDcData;

extern UCP_SOH_STATE_DATA ucpSohStateData;

//**************************************************************************************************
// Global Function Prototypes
//**************************************************************************************************

BATTERY_STATES checkForBatteryState(void);
i08 *getBatSerNumFromSedecal(void);
bool writeSohInputsToSdcard(void);
bool writeDutyCycleDataToSdcard(void);
bool appendToBatFileInSDCard(const i08 *, i08 * );
bool copyFileInSDCard(const i08 *srcFile, const i08 *destFile);
void initSdCardIOs(void);
bool getStatus(const i08 *);
bool readLastRowOfCsvFile(const i08 *);
#endif
