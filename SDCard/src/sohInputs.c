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
// FILENAME:        sohInputs.c
//
// SUMMARY:
// This module contains Input parameters of SOH API.
//
// REVISION HISTORY:
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Author              Date          Comments on this revision
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Manju. N            SEP/22/2023   Initial release
//
//**************************************************************************************************

//**************************************************************************************************
// Includes
//**************************************************************************************************
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "userControlPanelMI.h"
#include "config.h"
#include "sdCardFuncs.h"
#include "Softtimers.h"
#include "rtcApi.h"
#include "sohInputs.h"
//#include "SOH_test_v4.h"
#include "userControlPanelAPI.h"
#include "superCapChargerMI.h"
#include "ff.h"
#include "coMsgProcessing.h"
//**************************************************************************************************
// Local Definitions
//**************************************************************************************************
    
// UCP RTC 1 sec routine - every 1 seconds to update RTC
#define UCP_RTC_ONE_SEC_TIMER_PERIOD_MS        1000 

// Battery SOH routine - every 30 seconds to send CAN message to Sedecal
#define UCP_SOH_30SEC_TIMER_PERIOD_MS          5000 //30000 

//**************************************************************************************************
// Local Variables
//**************************************************************************************************
i16 dayOfYear, prevDayOfYear;
i08 *ApndStr1, *fStr;
volatile bool _ucpSohPostInitFlag = false;

char writeDataArray[256]="", readDataArray[256]="";
static bool isBmsReadSuccess = false;
bool _bUcpStateChanged = false;
//**************************************************************************************************
// Global Variables
//**************************************************************************************************
// Copy the updated data from UCP FE to ExtU_SOH_test_v4_T structure - TO DO (Manju)

// UCP calculated variables for SOH algorithm
UCP_SCC_CAN_DATA ucpSccCanData;

UCP_SOH_ALG_INPUTS UcpSohAlgInputData;

UCP_RUNNING_AVG_DATA ucpRungAvgData;
//**************************************************************************************************
// Local Prototypes
//**************************************************************************************************

//**************************************************************************************************
// Global Prototypes
//**************************************************************************************************
//****************************************************************************************
// Function:    InitSohTimers
// Parameters:  None
// Returns:     None
// Overview:    This is to initialize UCP timers for 1 second and 30 seconds
//****************************************************************************************
void InitSohTimers(void) 
{
    // RTC update routine
    InitSoftTimer(UCP_RTC_ONE_SEC_TIMER, UCP_RTC_ONE_SEC_TIMER_PERIOD_MS, false, &batteryOneSecCalc);

    // Sedecal CAN data read routine
    InitSoftTimer(UCP_SOH_30SEC_TIMER, UCP_SOH_30SEC_TIMER_PERIOD_MS, false, &batteryThirtySecCalc);    
}

//****************************************************************************************
// Function:    IsSdCardExists
// Parameters:  None
// Returns:     None
// Overview:    This is to give the status of SD card presence
//****************************************************************************************
bool IsSdCardExists(void)
{
    return IN_SD_CARD_DETECT;
}

//****************************************************************************************
// Function:    getRtcFromApp
// Parameters:  None
// Returns:     None
// Overview:    This is read RTC from Application after power on initialization.
//****************************************************************************************
UCP_RTC_CTRL getRtcFromApp(void)
{
    // test App RTC simulation
    appRtc.Year = 2024;
    appRtc.Month = 02; 
    appRtc.Day  = 02; 
    appRtc.Hour = 15; 
    appRtc.Min  = 06;
    appRtc.Sec  = 30;
    appRtc.mSec = 0;
    //
    
    // update SysRtc
    ProcessUcpRtcCtrl();
    
    appRtcSecCounter = appRtc.Sec + appRtc.Min*60 + appRtc.Hour*3600 + appRtc.Day*86400 + \
            (appRtc.Year-70)*31536000 + ((appRtc.Year-69)/4)*86400 - ((appRtc.Year-1)/100)*86400 \
            + ((appRtc.Year+299)/400)*86400;
    return appRtc;
}

//****************************************************************************************
// Function:    powerOnUcpSohInit
// Parameters:  None
// Returns:     None
// Overview:    Initialize the Battery SOH variable and call routines on Power On.
//****************************************************************************************
void powerOnUcpSohInit(void)
{    
    UCP_RTC_CTRL RtcData;
    BATTERY_STATES batState;
    
   // ClrWdt();
    
    // Init SD card hardware
    initSdCardIOs();
    
    // Save RTC read from IAS - TO DO
    RtcData = getRtcFromApp();

    // check for new Battery with Serial Number
    if(_ucpSohPostInitFlag == false)
    {  
        batState = checkForBatteryState();
        
        if((batState == BATTERY_STATE_VALID) || (batState == BATTERY_STATE_NEW))
        {
            _ucpSohPostInitFlag = true;
        }
    }  
    
    // start timers after power on init
    if(_ucpSohPostInitFlag == true)
    {
        ResetSoftTimer(UCP_RTC_ONE_SEC_TIMER);
        ResetSoftTimer(UCP_SOH_30SEC_TIMER);   
    }
    
    //EnableWDTimer(ENABLE_WDT);
}

//****************************************************************************************
// Function:    calcRunningAverage
// Parameters:  CBP_OP_STATE ucpState - UCP PCB FW current state
// Returns:     bool true - write status to SD card is success
//              bool false - write status to SD card is failed
// Overview:    Calculates running average of T1/T2/T3 using "Temp1" read from Sedecal 
//              and system state
//*****************************************************************************************
bool calcRunningAverage(CBP_OP_STATE ucpState)
{
    bool status;
    //Test with different states values
    CBP_OP_STATE testStates[] = {UCP_OFF, UCP_OFF, UCP_OFF, UCP_OFF, UCP_ON_IDLE, UCP_ON_IDLE, UCP_ON_IDLE, UCP_ON_IDLE, UCP_ON_CHRG_DISCHARG, UCP_ON_CHRG_DISCHARG, UCP_ON_CHRG_DISCHARG,UCP_ON_CHRG_DISCHARG,};
    static int stateInd = 0;
    ucpState = testStates[stateInd++];
    switch(ucpState)
    {
        case UCP_OFF: 
            if(ucpSohStateData.T1StateFlag == false)
            {
                ucpSohStateData.T1StateFlag = true;
            }
            
            // Increment Sample count since Battery is installed
            ucpSohStateData.T1SampleCnt++;
            status = writeSohInputsToSdcard();
            
            // Calculate Running average of T1 
            ucpRungAvgData.RunAvgT1Val = ((ucpRungAvgData.PrevT1AvgVal * (ucpSohStateData.T1SampleCnt-1)) + ucpSccCanData.CurrentTemp1Value)/ucpSohStateData.T1SampleCnt;
            status = writeSohInputsToSdcard();
            
            ucpRungAvgData.PrevT1AvgVal = ucpRungAvgData.RunAvgT1Val;
            break;

        case UCP_ON_CHRG_DISCHARG:   
            if(ucpSohStateData.T2StateFlag == false)
            {
                ucpSohStateData.T2StateFlag = true;
                ucpSohDcData.DCT2EntryTime  = appRtcSecCounter;
            }
            
            // Increment Sample count since Battery is installed
            ucpSohStateData.T2SampleCnt++;
            status = writeSohInputsToSdcard();
            
            //previousAverage * (index - 1) + currentNumber ) / index;
            // Calculate Running average of T1 
            ucpRungAvgData.RunAvgT2Val = ((ucpRungAvgData.PrevT2AvgVal * (ucpSohStateData.T2SampleCnt-1)) + ucpSccCanData.CurrentTemp1Value)/ucpSohStateData.T2SampleCnt;
            status = writeSohInputsToSdcard();
            
            ucpRungAvgData.PrevT2AvgVal = ucpRungAvgData.RunAvgT2Val;
            break;

        case UCP_ON_IDLE:
            if(ucpSohStateData.T3StateFlag == false)
            {
                ucpSohStateData.T3StateFlag = true;
                ucpSohDcData.DCT3EntryTime  = appRtcSecCounter;
            }
            
            // Increment Sample count since Battery is installed
            ucpSohStateData.T3SampleCnt++;
            status = writeSohInputsToSdcard();
            
            //previousAverage * (index - 1) + currentNumber ) / index;
            // Calculate Running average of T1 
            ucpRungAvgData.RunAvgT3Val = ((ucpRungAvgData.PrevT3AvgVal * (ucpSohStateData.T3SampleCnt-1)) + ucpSccCanData.CurrentTemp1Value)/ucpSohStateData.T3SampleCnt;
            status = writeSohInputsToSdcard();
            
            ucpRungAvgData.PrevT3AvgVal = ucpRungAvgData.RunAvgT3Val;
            break;

        default:
            break;                    
    }    
    
    return status;
}

//****************************************************************************************
// Function:    saveSOHInputData
// Parameters:  CBP_OP_STATE UcpPrevState
// Returns:     bool true - write status to SD card is success
//              bool false - write status to SD card is failed
// Overview:    Calculates running average of T1/T2/T3 using "Temp1" read from Sedecal 
//              and system state
//*****************************************************************************************
bool saveSOHInputData(CBP_OP_STATE UcpPrevState)
{    
    bool status = false;
    //Test
    CBP_OP_STATE testStates[] = {UCP_OFF, UCP_OFF, UCP_OFF, UCP_OFF, UCP_ON_IDLE, UCP_ON_IDLE, UCP_ON_IDLE, UCP_ON_IDLE, UCP_ON_CHRG_DISCHARG, UCP_ON_CHRG_DISCHARG, UCP_ON_CHRG_DISCHARG,UCP_ON_CHRG_DISCHARG,};
    static int stateInd = 0;
    UcpPrevState = testStates[stateInd++];
    
    switch (UcpPrevState)
    {            
        case UCP_OFF:      
            status = writeSohInputsToSdcard();
            ucpSohStateData.T1StateFlag = 0;           
            break;

        case UCP_ON_CHRG_DISCHARG: 
            ucpSohDcData.DCT2PartOfDay += (appRtcSecCounter - ucpSohDcData.DCT2EntryTime);
            status = writeSohInputsToSdcard();
            ucpSohDcData.DCT2EntryTime = 0;
            ucpSohStateData.T2StateFlag = 0; 
            break;

        case UCP_ON_IDLE:
            ucpSohDcData.DCT3PartOfDay += (appRtcSecCounter - ucpSohDcData.DCT3EntryTime);
            status = writeSohInputsToSdcard();
            ucpSohDcData.DCT3EntryTime = 0;
            ucpSohStateData.T3StateFlag = 0;
            break;

        default:
            break;
    }   
    
    return status;
}

//****************************************************************************************
// Function:    logT3inT2StateDataAtDayChange
// Parameters:  None
// Returns:     bool true - write status to SD card is success
//              bool false - write status to SD card is failed
// Overview:    This is to log UCP_ON_IDLE data in UCP_ON_CHRG_DISCHRG state 
//              when day is changed
//****************************************************************************************
bool logT3inT2StateDataAtDayChange(void)
{
    bool status = false;
    
    ucpRungAvgData.RunAvgOfDCT3 += ucpSohDcData.DutyCycleT3OfDay/86400;
    status = writeDutyCycleDataToSdcard();
    ucpSohDcData.DutyCycleT3OfDay = 0;
    
    return status;
}

//****************************************************************************************
// Function:    logT2inT3StateDataAtDayChange
// Parameters:  None
// Returns:     bool true - write status to SD card is success
//              bool false - write status to SD card is failed
// Overview:    This is to log UCP_ON_CHRG_DISCHRG data in UCP_ON_IDLE state 
//              when day is changed
//****************************************************************************************
bool logT2inT3StateDataAtDayChange(void)
{
    bool status = false;
    
    ucpRungAvgData.RunAvgOfDCT2 += ucpSohDcData.DutyCycleT2OfDay/86400;
    status = writeDutyCycleDataToSdcard();
    ucpSohDcData.DutyCycleT2OfDay = 0;
    
    return status;
}

//****************************************************************************************
// Function:    logT2StateDataAtDayChange
// Parameters:  None
// Returns:     bool true - write status to SD card is success
//              bool false - write status to SD card is failed
// Overview:    This is to log UCP_ON_CHRG_DISCHRG data when day is changed
//****************************************************************************************
bool logT2StateDataAtDayChange(void)
{
    bool status = false;
    
    ucpSohDcData.DutyCycleT2OfDay = (appRtcSecCounter - ucpSohDcData.DCT2EntryTime) + ucpSohDcData.DCT2PartOfDay;
    ucpRungAvgData.RunAvgOfDCT2 += ucpSohDcData.DutyCycleT2OfDay/86400;
    status = writeDutyCycleDataToSdcard();
    ucpSohDcData.DutyCycleT2OfDay = ucpSohDcData.DCT2PartOfDay = ucpSohDcData.DCT2EntryTime = 0;
    
    return status;
}

//****************************************************************************************
// Function:    logT2StateDataAtDayChange
// Parameters:  None
// Returns:     bool true - write status to SD card is success
//              bool false - write status to SD card is failed
// Overview:    This is to log UCP_ON_IDLE data when day is changed
//****************************************************************************************
bool logT3StateDataAtDayChange(void)
{
    bool status = false;
    
    ucpSohDcData.DutyCycleT3OfDay = (appRtcSecCounter - ucpSohDcData.DCT3EntryTime) + ucpSohDcData.DCT3PartOfDay;
    ucpRungAvgData.RunAvgOfDCT3 += ucpSohDcData.DutyCycleT3OfDay/86400;
    status = writeDutyCycleDataToSdcard();
    ucpSohDcData.DutyCycleT3OfDay = ucpSohDcData.DCT3PartOfDay = ucpSohDcData.DCT3EntryTime = 0;
    
    return status;
}

//****************************************************************************************
// Function:    calcLogDataAtDayChange
// Parameters:  None
// Returns:     bool true - write status to SD card is success
//              bool false - write status to SD card is failed
// Overview:    This is to log UCP_ON_CHRG_DISCHRG data in UCP_ON_CHRG_DISCHARG 
//              states data when day is changed
//****************************************************************************************
bool calcLogDataAtDayChange(CBP_OP_STATE OpState)
{
    bool status = false;
    
    //Test
    CBP_OP_STATE testStates[] = {UCP_OFF, UCP_OFF, UCP_OFF, UCP_OFF, UCP_ON_IDLE, UCP_ON_IDLE, UCP_ON_IDLE, UCP_ON_IDLE, UCP_ON_CHRG_DISCHARG, UCP_ON_CHRG_DISCHARG, UCP_ON_CHRG_DISCHARG,UCP_ON_CHRG_DISCHARG,};
    static int stateInd = 0;
    OpState = testStates[stateInd++];
    
    switch(OpState)
    {
        case UCP_ON_CHRG_DISCHARG:
            if(logT3StateDataAtDayChange() == true)
                if(logT2inT3StateDataAtDayChange() == true)
                    status = true;
                else
                    status = false;
            else
                    status = false;        
            break;
        
        case UCP_ON_IDLE:
            if(logT2StateDataAtDayChange() == true)           
                if(logT3inT2StateDataAtDayChange() == true)
                    status = true;
                else
                    status = false;
            else
                    status = false;               
            break;
        
        default:
            break;
    }
    return status;
}

//****************************************************************************************
// Function:    logDayChangeData
// Parameters:  None
// Returns:     bool true - write status to SD card is success
//              bool false - write status to SD card is failed
// Overview:    This is to log data of state data at day change
//****************************************************************************************
bool logDayChangeData(void)
{
    bool status = false;
  
    if(calcLogDataAtDayChange(_cbpStatus.OperationState) == true) 
        status = true;
    else
        status = false;
    
    return status;
}

//****************************************************************************************
// Function:    writeBmsDataToSDCard
// Parameters:  None
// Returns:     bool true - write status to SD card is success
//              bool false - write status to SD card is failed
// Overview:    This is to write SCC CAN data read from BMS and write to SD card
//****************************************************************************************
bool writeBmsDataToSDCard(void)
{
    bool status = false;
    static int Temp1=2201, RC=956, FC=987,soc=87, soh=89;
    
    ucpSccCanData.CurrentTemp1Value = Temp1++;//_sccBmsIndex4Data.Temp1;
            
    ucpSccCanData.RemCap = RC++;//_sccBmsIndex0Data.RemainingCapacity;

    ucpSccCanData.FullCap = FC++;//_sccBmsIndex1Data.FullCapacity;
    ucpSccCanData.SoC = soc++;//_sccBmsIndex1Data.BatterySOC;
    //_sccBmsIndex3Data.ProductionDate & _sccBmsIndex3Data.SwVersion

    ucpSccCanData.SoH = soh++;//_sccBmsIndex5Data.SOH;    

    memset(writeDataArray,0,256);
    sprintf(writeDataArray,"%lu,%ld,%ld,%ld,%ld,%ld\n", appRtcSecCounter, ucpSccCanData.CurrentTemp1Value, ucpSccCanData.RemCap,  ucpSccCanData.FullCap, ucpSccCanData.SoC, ucpSccCanData.SoH);

    if(appendToBatFileInSDCard(sccCanFile, writeDataArray) == true)
        status = true;
    else
        status = false;
    
    return status;
}

//****************************************************************************************
// Function:    readBatteryDataForCalc
// Parameters:  None
// Returns:     bool isBmsReadSuccess - BMS Data read is Successful
//              bool false -  BMS Data read is Unsuccessful
// Overview:    This is to read SCC BMS CAN data to calculate SOH inputs
//****************************************************************************************
bool readBatteryDataForCalc(void)
{   
    isBmsReadSuccess = false;
    
    if(!isBmsReadSuccess)
    {        
        // Send CAN messages to read data 
        /*gSlvForceTxPdoEvent(TPDO_SCC_BMS_INDEX_0_GET); // _sccBmsIndex0Data.RemainingCapacity

        gSlvForceTxPdoEvent(TPDO_SCC_BMS_INDEX_1_GET); //_sccBmsIndex1Data.FullCapacity & _sccBmsIndex1Data.BatterySOC
        gSlvForceTxPdoEvent(TPDO_SCC_BMS_INDEX_3_GET); // _sccBmsIndex3Data.ProductionDate & _sccBmsIndex3Data.SwVersion
        gSlvForceTxPdoEvent(TPDO_SCC_BMS_INDEX_4_GET); //_sccBmsIndex4Data.Temp1
        gSlvForceTxPdoEvent(TPDO_SCC_BMS_INDEX_5_GET); // _sccBmsIndex5Data.SOH (Full capacity / Factory capacity)

        gSlvForceTxPdoEvent(TPDO_SCC_BATT_CHARGER_SN_GET);*/
        
        // Copy the data to structure
        if(writeBmsDataToSDCard() == true)        
            isBmsReadSuccess =  true;
        else
           isBmsReadSuccess =  false;     
    }
    
    return isBmsReadSuccess;
}
//****************************************************************************************
// Function:    batteryOneSecCalc
// Parameters:  None
// Returns:     None
// Overview:    This is to calculate and update one second parameters
//****************************************************************************************
void batteryOneSecCalc(void)
{     
    bool status;
    
    //Increment RTC seconds counter by 1
    UpdateRtc();
    
    // Read CAN data
    if(readBatteryDataForCalc() == true)
    {
        // Convert CAN RTC(yyyy/month/day/hh/mm/ss) i16o yy/ddd/hh/mm/ss
        dayOfYear = RtcToDaysOfYear(appRtc.Year, appRtc.Month, appRtc.Day);

        // On power on save current day as previous day
        if(_ucpSohPostInitFlag == true)
            prevDayOfYear = dayOfYear;

        // if Day is changed, check this every 1 second 
        if(prevDayOfYear != dayOfYear)
        {
            if(logDayChangeData() == true)
                status = true;
            else
                status = false;
        }
    }
    else
    {
        status = false;
    }     
}

//****************************************************************************************
// Function:    readSccTemp1Value
// Parameters:  None
// Returns:     None
// Overview:    This is to read "Temp1" value from Sedecal CAN
//****************************************************************************************
long readSccTemp1Value(void)
{
    /* Send CAN message to Sedecal to get Temp1 value */
    gSlvForceTxPdoEvent(TPDO_SCC_BMS_INDEX_4_GET);
   
    return(_sccBmsIndex4Data.Temp1);
}

//****************************************************************************************
// Function:    batteryThirtySecCalc
// Parameters:  None
// Returns:     None
// Overview:    This is to calculate and update one second parameters
//****************************************************************************************
void batteryThirtySecCalc(void)
{       
    bool status = false;
    
    if(_bUcpStateChanged == true)
    {
        if(saveSOHInputData(_previousState) == true)
        {
            status = true;
            _bUcpStateChanged = false;
        }
        else
            status = false;
    }
    else 
    {
        if(calcRunningAverage(_cbpStatus.OperationState) == true)
            status = true;
        else
            status = false;
    }   
}

//****************************************************************************************
// Function:    calcAvgTempOfUcpOffState
// Parameters:  None
// Returns:     None
// Overview:    This is to calculate average Temp1 value using Temp1 at power down and power up
//****************************************************************************************
void calcAvgTempOfUcpOffState(void)
{    
    bool readCsvSts;
    u32 csvFileLastRtc;
    
    // Read SCC file from SD card
    readCsvSts = readLastRowOfCsvFile(sccCanFile);  
    
    // Copy RTC
    csvFileLastRtc = csvFileLastRowData[RTC]; 
    
    // Read last but one row to validate Last row is having latest RTC before power fail - TO DO
    
    // Copy Temp1
    ucpRungAvgData.LatestSDCTemp1 = csvFileLastRowData[TEMP1];  
    
    // Read Temp1 from Sedecal CAN
    ucpSccCanData.CurrentTemp1Value = readSccTemp1Value();    
    
    // Do average of them
    ucpRungAvgData.RunAvgT1Val = (ucpSccCanData.CurrentTemp1Value + ucpRungAvgData.LatestSDCTemp1)/2;
}


