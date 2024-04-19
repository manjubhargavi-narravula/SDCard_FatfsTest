/*
    (c) 2016 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
*/

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ff.h"
#include "sd_spi.h"
#include "sdCardFuncs.h"
//static FATFS drive;

 // Adjust the buffer size according to the number of bytes you want to read

FATFS fs;            // File system object
FIL file;            // File object
char buffer[BUFFER_SIZE];

unsigned charbw2;
FIL newFile2;
FATFS fatFs3;
FRESULT resOpenFile, resSdCardMnt;
unsigned int br;
unsigned char rdData[BUFFER_SIZE] = "";

char* rdStr="";
const char csvfilename[FILENAME_MAXLEN] = "HELLO15.csv";//"TestFile11.csv";
unsigned int bw1,br1;
FIL newFile1;
FATFS fatFs2;
FRESULT resOpenFile3, resSdCardMnt3;
char wrData14[BUFFER_SIZE] = "RTC,Temp1,Remaining Capacity,Full Capacity,SoC,SOH\n1234564890,2456,599,600,100,100\n1234564891,2234,600,600,99,100\n1234564892,2245,598,599,100,100\n";
char rdData11[BUFFER_SIZE] = "";
char rdBuf[BUFFER_SIZE]="";
char* rdStr3="";
char wrData11[BUFFER_SIZE] = "1234564890,2456,599,600,100,100\n1234564891,2234,600,600,99,100\n1234564892,2245,598,599,100,100\n";
char wrData1[BUFFER_SIZE];

char buffer2[BUFFER_SIZE], rdbuffer2[BUFFER_SIZE];
char  readSDCFile[FILENAME_MAXLEN];

const char csvfilename1[FILENAME_MAXLEN] = "SccCanDataLog3.csv";
bool flag=false;
 
bool appendToBatFileInSDC(const char *BatFilName, char *AppendStr )
{    
    unsigned int wca = 0;
    FIL appfile;
    FATFS fatFs;
    FRESULT fOpenSts, resSdCardMnt;
    bool res;
    
    clearBuffer(wrData11);
    resSdCardMnt = f_mount(&fatFs, "", 1);

    if( resSdCardMnt == FR_OK) /* Mount SD */ 
    {        
        fOpenSts = f_open(&appfile, BatFilName, FA_OPEN_APPEND | FA_WRITE);
        if(fOpenSts == FR_OK) 
        {            
            strcpy(wrData11, AppendStr);
            f_write(&appfile, wrData11, sizeof(wrData11), &wca);  
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
        res = false;
    }
    return res;
}

bool appendToBatFileInSDCard1(const char *BatFilName, char *AppendStr )
{    
    unsigned int wca = 0;
    FIL appfile;
    FATFS fatFs;
    FRESULT fOpenSts, resSdCardMnt;
    bool res;
    
    clearBuffer(wrData1);
    resSdCardMnt = f_mount(&fatFs, "", 1);

    if( resSdCardMnt == FR_OK) /* Mount SD */ 
    {        
        fOpenSts = f_open(&appfile, BatFilName, FA_OPEN_APPEND | FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
        if(fOpenSts == FR_OK) 
        {            
            strcpy(wrData1, AppendStr);
            f_write(&appfile, wrData1, sizeof(wrData1), &wca);  
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

void WriteDataTOSDC(const char *BatFilName)
{
    FRESULT res;
    FIL sdFile;
    bool status = false;
    FATFS fatFs;
    char wrData1App[256] = "";    
    FILINFO *dirFileInfo = 0;    
    
    clearBuffer(wrData1App);
    
    // Mount SD card
    res = f_mount(&fatFs, "", 1);

    if (res == FR_OK) 
    {
        res = f_stat(BatFilName, dirFileInfo);
        if(res == FR_OK)
        {
            res = f_open(&sdFile, BatFilName, FA_OPEN_APPEND | FA_OPEN_ALWAYS | FA_READ | FA_WRITE);

            if(res == FR_OK) 
            {
                status = true;                    
            }  
            else
            {
                status = false;
                return;
            }             
        }
        else
        {
            res = f_open(&sdFile, BatFilName, FA_OPEN_APPEND | FA_OPEN_ALWAYS | FA_READ | FA_WRITE);

            if(res == FR_OK) 
            {
                status = true;
                appendToBatFileInSDCard1(BatFilName, wrData14);
                clearBuffer(wrData1App);
            }
            else
            {
                status = false;
                return;
            }   
        }
    }
    else
    {
        status = false;
        return;
    }
    
    if(status == true)
    {
        appendToBatFileInSDCard1(BatFilName, wrData11);
    }
    
//    return status;
}

void csvTestSDCWrite1(const char *filename)
{        
    static bool wrflag = false;
    static bool MountSts = false;
    
    if (MountSts == false)
    {
        resSdCardMnt = f_mount(&fatFs2, "", 1);        
        MountSts = true;
    }
    
    if( resSdCardMnt3 == FR_OK) /* Mount SD */ 
    {         
        //resOpenFile3 = f_open(&newFile1, "TestFile11.csv", FA_OPEN_APPEND | FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
        resOpenFile3 = f_open(&newFile1, filename, FA_OPEN_APPEND | FA_OPEN_ALWAYS | FA_READ | FA_WRITE);

        if(resOpenFile3 == FR_OK) 
        {
            if(wrflag == false)
            {
                wrflag =  true;

                if(FR_OK == f_write(&newFile1, wrData14, sizeof(wrData14), &bw1))
                {
                    f_close(&newFile1); /* Close the file */     
                }
            }
            else
            {
                if(FR_OK == f_write(&newFile1, wrData11, sizeof(wrData11), &bw1))
                {
                    f_close(&newFile1); /* Close the file */     
                }
            }
        }
    }
}

void csvTestSDCRead1(const char *filename)
{        
    bool confirm = false;
    
    //char token[5];
    char *token;
    char btoken[5];
    resSdCardMnt = f_mount(&fatFs2, "", 1);
    // Read the current byte
    char currentChar;
    UINT bytesRead;
    long position;
    char tokenbuf[100];
    
    // open file to read
    resOpenFile3 = f_open(&newFile1, filename, FA_OPEN_ALWAYS | FA_READ);
        
    if(resOpenFile3 == FR_OK) 
    {

        if(FR_OK == f_read(&newFile1, rdData11, sizeof(rdData11), &br1))
        {   
            f_close(&newFile1); /* Close the file */     
        }        
    }
}

void csvTestSDCRead1LastRow(const char *filename)
{        
    bool MntSts = false;
    FRESULT result;
    uint16_t readBytes; 
    int filesize;
    
    if(MntSts == false)
    {
        // Mount the SD card
        result = f_mount(&fatFs2, "", 0);
        MntSts = true;
    }
    
    clearBuffer(rdData11);
    clearBuffer(rdBuf);
    
    // open file to read
    resOpenFile3 = f_open(&newFile1, filename, FA_OPEN_ALWAYS | FA_READ);        
 
    if(resOpenFile3 == FR_OK)
    {
        if(FR_OK == f_read(&newFile1, rdData11, sizeof(rdData11), &br1))
        {
            filesize = f_size(&newFile1);
            result = f_lseek(&newFile1, 10);  

            if(FR_OK == f_read(&newFile1, rdBuf, sizeof(rdBuf), &readBytes))
            {   
                f_close(&newFile1); /* Close the file */     
            }      
        }
    }
}

void csvTestSDCRead1LastBytes(const char *filename)
{        
    bool confirm = false;
    
    //char token[5];
    char *token;
    char btoken[5];
    // Read the current byte
    char currentChar;
    UINT bytesRead;
    long position;
    char tokenbuf[100];
    bool MntSts = false;
    
    FRESULT result;

    clearBuffer(rdBuf); 
    if(MntSts == false)
    {
        // Mount the SD card
        result = f_mount(&fatFs2, "", 0);
        MntSts = true;
    }
   
    if( result == FR_OK) /* Mount SD */ 
    {         
        // Open the file
        result = f_open(&newFile1, filename, FA_READ);
        
        if( result == FR_OK) /* Mount SD */  {
        
            int filesize = f_size(&newFile1);
            int filesize1 = f_tell(&newFile1);
            
            result = f_lseek(&newFile1, f_size(&newFile1) - 100);
            memcpy(newFile1.buf, rdData11, sizeof(newFile1.buf));
            
            if (result == FR_OK) {

                if(FR_OK == f_read(&newFile1, rdBuf, sizeof(rdBuf), &br1))
                {   
                    f_close(&newFile1); /* Close the file */     
                }
            }
        }
    }
    
    f_close(&newFile1); /* Close the file */    
}

void readLastBytes(const char *filename, uint32_t numBytes) {
    FRESULT result;
    UINT bytesRead;

    // Mount the SD card
    result = f_mount(&fs, "", 0);
    if (result != FR_OK) {
        // Handle mount error
        return;
    }

    // Open the file
    result = f_open(&file, filename, FA_READ);
    if (result != FR_OK) {
        // Handle file open error
        f_mount(0, "", 0);  // Unmount the file system
        return;
    }

    long unsigned int filesize = f_size(&file);
    // Move to the end of the file
    result = f_lseek(&file, f_size(&file) - numBytes);
    if (result != FR_OK) {
        // Handle seek error
        f_close(&file);  // Close the file
        f_mount(0, "", 0);  // Unmount the file system
        return;
    }

    // Read the last N bytes into the buffer
    result = f_read(&file, buffer, numBytes, &bytesRead);
    if (result != FR_OK) {
        // Handle read error
        f_close(&file);  // Close the file
        f_mount(0, "", 0);  // Unmount the file system
        return;
    }
    char *lastNewline = strrchr(buffer, '\n');

    if (lastNewline != 0) 
    {
        // Find the next newline character after the last newline
        char *nextNewline = lastNewline-1;// = strchr(lastNewline - 1, '\n');

        while (nextNewline > buffer && *nextNewline != '\n') {
            //printf("prevNewline: %s\n", nextNewline);
            nextNewline--;
        }

        if (nextNewline != 0) 
        {
            // Calculate the length of the substring between the last and next newline
            size_t length = lastNewline - (nextNewline + 1);

            // Create a buffer to store the substring
            char substring[BUFFER_SIZE];

            // Copy the substring to the buffer
            strncpy(substring, nextNewline + 1, length);

            // Null-terminate the substring
            substring[length] = '\0';

            // Tokenize the substring using strtok

            char *commaDelimiter = ",";

            // Use strtok to split the substring by commas
            char *token = strtok(substring, commaDelimiter);

            while (token != 0) 
            {
                // Now, 'token' contains each value separated by commas
                //printf("Token: %s\n", token);

                // Get the next token
                token = strtok(0, commaDelimiter);
            }
        }
    }
    // Close the file
    f_close(&file);

    // Unmount the file system
    f_mount(0, "", 0);

    // Null-terminate the buffer
    buffer[bytesRead] = '\0';
}
 
void csvfatfsOperations(void)
{  
   // csvTestSDCWrite1(csvfilename);
   // csvTestSDCRead1(csvfilename);
    //csvTestSDCRead1("HELLO15.csv");
    //csvTestSDCRead1LastBytes("HELLO15.csv");
    csvTestSDCRead1LastRow("Hello1.csv");
    csvTestSDCRead1LastRow("Hello2.csv");
    
//    csvTestSDCRead1LastBytes("csvTestFile6.csv");
    //csvTestSDCRead1LastBytes("SccCanDataLog3.csv");
    //csvTestSDCRead1LastBytes("SccCanDataLog.csv");
   // csvTestSDCRead1LastBytes("SccCanDataLog2.csv");
   // csvTestSDCRead1LastBytes("csvTestFile5.csv");
   // csvTestSDCRead1LastBytes("csvTestFile1.csv");
    //csvTestSDCRead1LastBytes("SccCanDataLog1.csv");
    //csvTestSDCRead1LastBytes(" csvTestFile4.csv");
    
    flag=true;
}

void FatFsDemo_Tasks(void)
{
    UINT actualLength;
    FATFS fs;
    char data[] = "Hello World Again!";
    if( SD_SPI_IsMediaPresent() == true)
    {
        return;
    }

    //if (f_mount(&drive,"0:",1) == FR_OK)
    FRESULT res = f_mount(&fs, "", 0); // Mount the default drive
    if (res != FR_OK) {
        // Handle mount error
        return;
    }
    else {
        //    if (f_open(&file, "HELLO1.TXT", FA_WRITE | FA_CREATE_NEW ) == FR_OK)
        res = f_open(&file, "HELLO14.TXT", FA_WRITE | FA_CREATE_ALWAYS);
        if (res != FR_OK) {
            // Handle file open error
            f_mount(0, "", 0); // Unmount drive if failed to open the file
            return;
        }
        else
        {
            f_write(&file, data, sizeof(data)-1, &actualLength );
            f_close(&file);
        }
    }
}



