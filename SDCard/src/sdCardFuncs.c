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
// FILENAME:        sdCardFuncs.c
//
// SUMMARY:
// This module contains SD card battery functionalities to calculates SOH Input parameters.
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
#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <string.h>
#include "ff.h"
#include "sdspi.h"
#include "softTimers.h"
#include "rtcApi.h"
#include "sohInputs.h"
#include "sdCardFuncs.h"
#include "delay.h"
#include "ioPorts.h"
//**************************************************************************************************
// Local Definitions
//**************************************************************************************************

//**************************************************************************************************
// Local Variables
//**************************************************************************************************
// Duty cycle running average values
 
float RunAvgOfDCT2, RunAvgOfDCT3;

UCP_SOH_DC_DATA ucpSohDcData;

UCP_SOH_STATE_DATA ucpSohStateData;
        
//const i08 SedecalNewBatSerNum[FILENAME_MAXLEN]= "ABCD100124XYZ";
i08 SedecalNewBatSerNum[FILENAME_MAXLEN] = "ABCD-26MAR24-XYZ";

i08 *NewSedBatFileName;
i08 sccCanFile[FILENAME_MAXLEN], sohInputFile[FILENAME_MAXLEN], dutyCycleFile[FILENAME_MAXLEN];
i08 wrData[256] = " ";

i08 *AppendStr;
FATFS fatFs;
FRESULT resOpen;
FSIZE_t fsize;

FRESULT resApp;
FSIZE_t fsize;
i08 csvFileLastRowData[READ_ROW_LENGTH];
u32 parseStringArray[MAXPARAMS];
i08 csvLastRowOnly[READ_ROW_LENGTH];
//**************************************************************************************************
// Global Variables
//**************************************************************************************************

//**************************************************************************************************
// Local Prototypes
//**************************************************************************************************

//**************************************************************************************************
// Global Prototypes
//**************************************************************************************************
//****************************************************************************************
// Function:    initSdCardIOs
// Parameters:  None
// Returns:     None
// Overview:    Initialize the SPI I/Os to interface with SD card
//****************************************************************************************
void initSdCardIOs(void)
{       
    // Turn ON to enable power to SD card   
    OUT_EN_SUPPLY_SD_CARD = HI;
}

//****************************************************************************************
// Function:    getSDCardMountStatus
// Parameters:  None
// Returns:     false - if not mounted
//              true - if mounted
// Overview:    This is to get SD card mounting status
//****************************************************************************************
bool getSDCardMountStatus(void)
{
    FATFS SdCardMnt;
    FRESULT resSdCardMnt; 
    resSdCardMnt = f_mount(&SdCardMnt, "", 1);
    bool res;
    
    if(resSdCardMnt == FR_OK)
        res = true;
    else
        res = false;
    
    return res;
}

//****************************************************************************************
// Function:    getSDCardChecksum
// Parameters:  None
// Returns:     false - if checksum is fail/not valid
//              true - if checksum is valid
// Overview:    This is to get/calculate SD card checksum
//****************************************************************************************
bool getSDCardChecksum(void)
{
    return VALID;
}

//****************************************************************************************
// Function:    getStatus
// Parameters:  u08 *Name - File/Directory name
// Returns:     false - if file is not exists
//              true - if file is exists
// Overview:    This is to get file existence status in SD card
//****************************************************************************************

bool getStatus(const i08 *Name)
{
    bool res;
    FILINFO *dirFileInfo = 0;    
    
    FRESULT resDir = f_stat(Name, dirFileInfo);

    if(resDir > FR_OK)
        res = false;  //file not exists
    else
        res = true; //file exists
    
    return res;
}

//****************************************************************************************
// Function:    createNewBatFileInSDCard
// Parameters:  u08 *BatFilName
// Returns:     false - if file is created
//              true - if file is not created
// Overview:    This is to create new battery text file in SD card
//****************************************************************************************
bool createNewBatFileInSDCard(const i08 *BatFilName)
{        
    FIL file;
    FRESULT resNewFile;
    bool res;
    
    resNewFile = f_open(&file, BatFilName, FA_CREATE_NEW | FA_CREATE_ALWAYS | FA_READ | FA_WRITE);

    if(resNewFile == FR_OK) 
    {
        /* Close the write file */ 
        f_close(&file); 
        res = true;
    }        
    else
    {
        res = false;
    }  

    return res;
}

//****************************************************************************************
// Function:    appendToBatFileInSDCard
// Parameters:  u08 *BatFilName, i08 *AppendSt
// Returns:     false - if string is appended to file
//              true - if string is not appended to file
// Overview:    This is to append given string to the text file in SD card
//****************************************************************************************
bool appendToBatFileInSDCard(const i08 *BatFilName, i08 *AppendStr )
{    
    u16 wca = 0;
    FIL appfile;
    FATFS fatFs;
    FRESULT fOpenSts, resSdCardMnt;
    bool res;
    
    memset(wrData,0,sizeof(wrData));
    resSdCardMnt = f_mount(&fatFs, "", 1);

    if( resSdCardMnt == FR_OK) /* Mount SD */ 
    {        
        fOpenSts = f_open(&appfile, BatFilName, FA_OPEN_APPEND | FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
        if(fOpenSts == FR_OK) 
        {            
            strcpy(wrData, AppendStr);
            f_write(&appfile, wrData, sizeof(wrData), &wca);  
            f_close(&appfile); // Close the file
            res = true;
        }
        else
        {
            // Throw exception or log that BatFilName not exists in SD card given path
            res = false;
        }
    }
    else
    {
        //printf("\r\n SD card not mounted \r\n");      
        res = false;
    }
    return res;
}

//****************************************************************************************
// Function:    copyFileInSDCard
// Parameters:  u08 *BatFilName, i08 *AppendSt
// Returns:     false - if string is appended to file
//              true - if string is not appended to file
// Overview:    This is to append given string to the text file in SD card
//****************************************************************************************
bool copyFileInSDCard(const i08 *srcFile, const i08 *destFile)
{
    FIL file;
    FATFS sdCardmnt;
    BYTE lastRowCsvData[255];
    FRESULT res;
    bool status;
    
    u16 rdBytes = 0, wrBytes = 0;

    f_mount(&sdCardmnt, "", 1);

    res = f_open(&file, (const TCHAR*)srcFile, FA_READ | FA_OPEN_EXISTING);
    
    if (res)
    {
        f_mount(0, "", 0);
        status = false;
    }

    f_read(&file, lastRowCsvData, sizeof(lastRowCsvData), &rdBytes);  /* Read a chunk of source file */
    f_close(&file);

    if(rdBytes) 
    {
        res = f_open(&file, (const TCHAR*)destFile, FA_WRITE | FA_CREATE_ALWAYS);
        if (res) 
        {
            f_mount(0, "", 0);
            status = false;
        }

        f_write(&file, lastRowCsvData, rdBytes, &wrBytes); /* Write it to the destination file */
        f_close(&file);

        if(!wrBytes)
        {
            f_mount(0, "", 0);
            status = false;
        }
    }
    
    //unmount SD card
    f_mount(0, "", 0);
    
    return status;
}

//****************************************************************************************
// Function:    deleteFileInSDCard
// Parameters:  u08 *FilName
// Returns:     false - if file is deleted
//              true - if file is not deleted
// Overview:    This is to delete "FilName" file in SD card
//****************************************************************************************
FRESULT deleteFileInSDCard(const i08 *FilName)
{
    FRESULT delSts,resSdCardMnt;
    FATFS fatFs;
    
    resSdCardMnt = f_mount(&fatFs, "", 1);

    if( resSdCardMnt == FR_OK) /* Mount SD */ 
    {
        delSts = f_unlink(FilName);
    }
    else
    {
        
    }
    
    return delSts;
}

//****************************************************************************************
// Function:    renameFileInSDCard
// Parameters:  const u08 *orgFile
//              const u08 *renameFile
// Returns:     false - if file is deleted
//              true - if file is not deleted
// Overview:    This is to rename "orgFile" to "renameFile" in SD card
//****************************************************************************************
void renameFileInSDCard(const i08 *orgFile, const i08 *renameFile)
{
    FRESULT resSdCardMnt;
    FATFS fatFs;
    
    resSdCardMnt = f_mount(&fatFs, "", 1);

    if( resSdCardMnt == FR_OK) /* Mount SD */ 
    {
        f_rename(orgFile, renameFile); 
    }
    else
    {
        
    }
}

//****************************************************************************************
// Function:    writeSohInputsToSdcard
// Parameters:  None
// Returns:     bool true - write status to SD card is success
//              bool false - write status to SD card is failed
// Overview:    This is to write SOH input data to the SD card
//****************************************************************************************
bool writeSohInputsToSdcard(void)
{
    FRESULT res;
    bool status = false;
    FATFS fatFs;
    i08 wrDataApp[256] = "";    
    FILINFO *dirFileInfo = 0;  
    
    memset(wrDataApp, 0, 256);
    
    // Mount SD card
    res = f_mount(&fatFs, "", 1);

    if (res == FR_OK) 
    {       
        // Check SOO Inputs file is exists or not
        res = f_stat(sohInputFile, dirFileInfo);
        
        //if not exists create file or append data to file
        if(res != FR_OK)
        {
            if(res == FR_OK)
            {
                status = true;
                if(createNewBatFileInSDCard(sohInputFile) == true)
                {                            
                    status = true;
                    if(appendToBatFileInSDCard(sohInputFile, "RTC,T1 Value,T1 Sample Count,T2 Value,T2 Sample Count,T3 Value,T3 Sample Count\n") == true)
                        status = true;
                    else
                        status = false;
                }
                else
                    status = false;
            }
            else
                status = false;
        }
        else
        {
            memset(wrDataApp, 0, 256);
            sprintf(wrDataApp, "%ld,%ld,%ld,%ld,%ld,%ld,%ld\n",appRtcSecCounter,ucpRungAvgData.RunAvgT1Val,\
                    ucpSohStateData.T1SampleCnt,ucpRungAvgData.RunAvgT2Val,\
                    ucpSohStateData.T2SampleCnt,ucpRungAvgData.RunAvgT3Val,\
                    ucpSohStateData.T3SampleCnt);
            if(appendToBatFileInSDCard(sohInputFile, wrDataApp) == true)
                status = true;
            else
                status = false;    
        }
    }
    return status;
}

//****************************************************************************************
// Function:    writeDutyCycleDataToSdcard
// Parameters:  None
// Returns:     bool true - write status to SD card is success
//              bool false - write status to SD card is failed
// Overview:    This is to write Duty cycle data to SD card.
//****************************************************************************************
bool writeDutyCycleDataToSdcard(void)
{
    FRESULT res;
    FIL sdFile;
    bool status = false;
    FATFS fatFs;
    i08 wrDataApp[256] = "";    
    FILINFO *dirFileInfo = 0;  
    
    memset(wrDataApp, 0, 256);
    
    // Read Battery Serial Number from Sedecal
    NewSedBatFileName = getBatSerNumFromSedecal();
        
    sprintf(dutyCycleFile,"%s/DutyCycleData.csv", NewSedBatFileName);
    
    // Mount SD card
    res = f_mount(&fatFs, "", 1);

    if (res == FR_OK) 
    {       
        res = f_stat(dutyCycleFile, dirFileInfo);
        if(res == FR_OK)
        {
            res = f_open(&sdFile, sohInputFile, FA_OPEN_APPEND | FA_OPEN_ALWAYS | FA_READ | FA_WRITE);

            if(res != FR_OK) 
            {
                status = true;
                appendToBatFileInSDCard(sohInputFile, "RTC,T2 Duty Cycle Of Day,T2 Run Avg Duty Cycle, T3 Duty Cycle,\n");
            }
            memset(wrDataApp, 0, 256);
            sprintf(wrDataApp, "%ld,%ld,%ld,%ld,%ld\n",appRtcSecCounter, 
                    ucpSohDcData.DutyCycleT2OfDay, ucpRungAvgData.RunAvgOfDCT2, 
                    ucpSohDcData.DutyCycleT3OfDay,ucpRungAvgData.RunAvgOfDCT3);
            appendToBatFileInSDCard(sohInputFile, wrDataApp);    
            status = true;
        }
        else
            status = false;
    }
    else
        status = false;
                
    return status;
}

//***************************************************************************************
// Function:    readLastRowOfCsvFile
// Parameters:  filename - csv filename
// Returns:     None
// Description: Read last row of CSV file from SD Card
//***************************************************************************************
bool readLastRowOfCsvFile(const i08 *filename) 
{
    FIL file;            // File object
    FRESULT result;
    UINT bytesRead;
    u32 numBytes;
    
    numBytes = READ_ROW_LENGTH;
#if 0
    // Mount the SD card
    result = f_mount(&fs, "", 0);
    if (result != FR_OK) {
        // Handle mount error
        return false;
    }
#endif
    // Open the file
    result = f_open(&file, filename, FA_READ);
    if (result != FR_OK) {
        // Handle file open error
        f_mount(NULL, "", 0);  // Unmount the file system
        return false;
    }

    if (f_size(&file) != 0)
    {
    //    long unsigned int filesize = f_size(&file);
        // Move to the end of the file
        result = f_lseek(&file, f_size(&file) - numBytes);
        if (result != FR_OK) {
            // Handle seek error
            f_close(&file);  // Close the file
            f_mount(NULL, "", 0);  // Unmount the file system
            return false;
        }

        // Read the last N bytes into the csvFileLastRowData
        result = f_read(&file, csvFileLastRowData, numBytes, &bytesRead);
        if (result != FR_OK) {
            // Handle read error
            f_close(&file);  // Close the file
            f_mount(NULL, "", 0);  // Unmount the file system
            return false;
        }
        i08 *lastNewline = strrchr(csvFileLastRowData, '\n');

        if (lastNewline != NULL) 
        {
            // Find the next newline i08acter after the last newline
            i08 *nextNewline = lastNewline-1;// = strchr(lastNewline - 1, '\n');

            while (nextNewline > csvFileLastRowData && *nextNewline != '\n') {
                //printf("prevNewline: %s\n", nextNewline);
                nextNewline--;
            }

            if (nextNewline != NULL) 
            {
                // Calculate the length of the substring between the last and next newline
                size_t length = lastNewline - (nextNewline + 1);

                // Create a csvFileLastRowData to store the substring
                i08 substring[READ_ROW_LENGTH];

                // Copy the substring to the csvFileLastRowData
                strncpy(substring, nextNewline + 1, length);

                // Null-terminate the substring
                substring[length] = '\0';

                // Tokenize the substring using strtok

                i08 *commaDelimiter = ",";

                // Use strtok to split the substring by commas
                i08 *token = strtok(substring, commaDelimiter);

                while (token != NULL) 
                {
                    // Now, 'token' contains each value separated by commas
                    //printf("Token: %s\n", token);

                    // Get the next token
                    token = strtok(NULL, commaDelimiter);
                }
            }
        }
        // Close the file
        f_close(&file);

        // Unmount the file system
        f_mount(NULL, "", 0);

        // Null-terminate the csvFileLastRowData
        csvFileLastRowData[bytesRead] = '\0';

        return true;
    }
    else
        return false;
}

//***************************************************************************************
// Function:    convertStringToDecimal
// Parameters:  StringValue - String to be converted into long value
// Returns:     u32 - long value which holds in string
// Description: Read "sdparam" data from SD Card
//***************************************************************************************
u32 convertStringToDecimal(i08 *StringValue)
{
    u32 retValue;
    // Base is 10 because we are converting to integer.
    retValue = strtol(StringValue, NULL, 10);
    //long long t = atoll(s);
    return retValue;
}

//****************************************************************************************
// Function:    getBatSerNumFromSedecal
// Parameters:  None
// Returns:     None
// Overview:    This is to get Serial number of the Battery
//****************************************************************************************
i08 *getBatSerNumFromSedecal(void)
{
    // CAN message format - 0x00 0x03 0x02 0x00 0x00 0x12 0x19 - NewSedBatFileName (7 bytes))
    return SedecalNewBatSerNum;
}

//****************************************************************************************
// Function:    copyStringBetweenNewlines
// Parameters:  i08 source[], i08 destination[]
// Returns:     None
// Overview:    This is to copy source array into destination array
//****************************************************************************************
void copyStringBetweenNewlines(i08 source[], i08 destination[]) 
{
    i16 i, j = 0;
    i16 start_index = -1;
    i16 end_index = -1;

    // Find the indices of the first and second newline i08acters
    for (i = 0; source[i] != '\0'; i++) {
        if (source[i] == '\n') {
            if (start_index == -1) {
                start_index = i + 1; // Start copying after the first newline
            } else {
                end_index = i; // Copy until the second newline
                break;
            }
        }
    }

    // If both newline i08acters are found, copy the string between them
    if (start_index != -1 && end_index != -1) {
        for (i = start_index; i < end_index; i++) {
            destination[j++] = source[i];
        }
    }

    // Null-terminate the destination array
    destination[j] = '\0';
}

//****************************************************************************************
// Function:    parseString
// Parameters:  i08 *inputString
// Returns:     None
// Overview:    This is to convert string into long values
//****************************************************************************************
void parseString(i08 *inputString)
{
    SCCCAN_PARAMS parmInd = RTC;    
    // Tokenizing the input string based on commas
    i08 *token = strtok(inputString, ",");
    u32 numbers[100];
    i16 count = 0;
    
    // Removing newline i08acter if present
    if (inputString[strlen(inputString) - 1] == '\n')
        inputString[strlen(inputString) - 1] = '\0';


    // Converting tokens to i16egers and storing in an array
    while (token != NULL) {
        numbers[count++] = atol(token);
        token = strtok(NULL, ",");
    }

    // Pri16ing the separated i16egers
    for (i16 ind = 0; ind < count; ind++) {
        parseStringArray[parmInd++]= numbers[ind];
    }
}

//****************************************************************************************
// Function:    ReadLastRowAndCopyData
// Parameters:  None
// Returns:     None
// Overview:    This is to read last row of SCC CAN data saved CSV file and 
//              converts string to integer values (long) and then copy the values
//              into an u32 array
//****************************************************************************************
void ReadLastRowAndCopyData(void)
{
    // Read last but one row to validate Last row is having latest RTC before power fail - TO DO
    copyStringBetweenNewlines(csvFileLastRowData, csvLastRowOnly);
    //copyStringBetweenNewlines(input, output);

    // Split data separated by ',' and then convert into integer/long
    parseString(csvLastRowOnly);   
}

//****************************************************************************************
// Function:    ReadSccCanDataFromSDCard
// Parameters:  None
// Returns:     None
// Overview:    This is to read SCC CAN data from SD card csv file and copied to local 
//              variables for use
//****************************************************************************************
void ReadSccCanDataFromSDCard(void)
{       
   readLastRowOfCsvFile(sccCanFile);  
   ReadLastRowAndCopyData();
   ucpRungAvgData.LatestSDCTemp1 = 0;
}

//****************************************************************************************
// Function:    ReadSohInputDataFromSDCard
// Parameters:  None
// Returns:     None
// Overview:    This is to read SOH input data from SD card csv file and copied to local 
//              variables for use
//****************************************************************************************
void ReadSohInputDataFromSDCard(void)
{       
    readLastRowOfCsvFile(sohInputFile);  
   
    ucpRungAvgData.RunAvgT1Val = 0;
    ucpSohStateData.T1SampleCnt = 0;
    ucpRungAvgData.RunAvgT2Val = 0;
    ucpSohStateData.T2SampleCnt = 0;
    ucpRungAvgData.RunAvgT3Val = 0;
    ucpSohStateData.T3SampleCnt = 0;
}

//****************************************************************************************
// Function:    ReadDutyCycleDataFromSDCard
// Parameters:  None
// Returns:     None
// Overview:    This is to read Duty Cycle data from SD card csv file and copied to local 
//              variables for use
//****************************************************************************************
void ReadDutyCycleDataFromSDCard(void)
{       
    readLastRowOfCsvFile(dutyCycleFile);  
   
    ucpSohDcData.DutyCycleT2OfDay = 0;
    ucpRungAvgData.RunAvgOfDCT2 = 0;
    ucpSohDcData.DutyCycleT3OfDay = 0;
    ucpRungAvgData.RunAvgOfDCT3 = 0;
}

//****************************************************************************************
// Function:    checkForBatteryState
// Parameters:  None
// Returns:     BATTERY_STATES
// Overview:    This is to get Battery state
//****************************************************************************************
BATTERY_STATES checkForBatteryState(void)
{
    BATTERY_STATES batState;
    FRESULT res;
    FATFS fatFs; 
    
    // Read Battery Serial Nmber from Sedecal with 0x19 command and no CAN error
    // it should be 12 i08acters ends with NULL
    bool sedecalCANSts = true;
           
    // Sedecal CAN read is successful
    if(sedecalCANSts == true)
    {
        // Read Battery Serial Number from Sedecal
        NewSedBatFileName = getBatSerNumFromSedecal();

        // Mount SD card
        res = f_mount(&fatFs, "", 1);
        
        if (res == FR_OK) 
        {
            if(getStatus(NewSedBatFileName) == true) // if exists
            {
                ;
            }
            else
            {
                // create folder with Battery serial number
                res = f_mkdir(NewSedBatFileName);
            }
            
            if (res == FR_OK) 
            {            
                // check for SD card checksum
                if(getSDCardChecksum() == VALID)
                {
                    sprintf(sccCanFile,"%s/SCCCANData.csv", NewSedBatFileName);
                    
                    // check for BatterySN# folder exists in SD card
                    if(getStatus(sccCanFile) == true) // if exists
                    {
                        batState = BATTERY_STATE_VALID;   // check for other Battery states
                        ReadSccCanDataFromSDCard();
                        ReadSohInputDataFromSDCard();
                        ReadDutyCycleDataFromSDCard();
                        calcAvgTempOfUcpOffState();                    
                    }
                    else
                    {
                        // Create new file in a folder
                        if(createNewBatFileInSDCard(sccCanFile) == true)
                        {                          
                            appendToBatFileInSDCard(sccCanFile, "RTC,Temp1,Remaining Capacity,Full Capacity,SoC,SOH\n");
                            batState = BATTERY_STATE_NEW;   // Log Battery state as INITIAL for new battery
                        }
                        else
                            batState = BATTERY_STATE_FILE_CREATE_ERROR;
                    }
                }
                else
                    batState = BATTERY_STATE_CHKSUM_ERR;
            }
            else
                batState = BATTERY_STATE_FOLDER_ERR;
        }
        else
            batState = BATTERY_STATE_SDCARD_MNT_ERR;
    }
    else
        batState = BATTERY_STATE_SEDECAL_ERR;
    
    return batState;
}
