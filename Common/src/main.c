//**************************************************************************************************
// Includes
//**************************************************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <xc.h>                 // Device header file (ClrWdt)
#include <delay.h>
#include "configBits.h"         // Writes the configuration registers
#include "typedefs.h"           // Necessary definitions
#include "ff.h"
//**************************************************************************************************
// MAIN Function
//**************************************************************************************************
    
char SDCardFile1[50] = "Read08042024TestFileLen.csv";
char SDCardFile2[50] = "bkup_Read08042024TestFileLen.csv";

char SDCardFile3[50] = "1_bkup_Read08042024TestFileLen.csv";

int intVal = 2568;
float fVal = 23.586;
double dVal = 2566.669999;

u16 bw;
FIL newFile;
FATFS fatFs;
FRESULT resOpen, resSdCardMnt;
FSIZE_t fsize;
//static bool _fileOpenFlag = false;
char wrData[256]="";
    
void InitExpSDCIOports(void)
{  
  // setup the SPI2 port
    LATC = 0x0000;
    TRISC = 0xFFFF;
    _TRISC13 = PIN_IN;

    LATG = 0x0000;
    TRISG = 0xFFFF;
    _TRISG6 = PIN_OUT; // SPI2 SCK2 - RG6 - Output
    _TRISG7 = PIN_IN; // SPI2 SDI2 - RG7 - Input  
    _TRISG8 = PIN_IN; // SPI2 SDO2 - RG8 - Output
    _TRISG9 = PIN_OUT; // SPI2 CS2 - RG9 - Output
     
}

void TestSDCardWrite(char *SDCardFile)
{    
    resSdCardMnt = f_mount(&fatFs, "", 1);

    if( resSdCardMnt == FR_OK) /* Mount SD */ 
    {     
        resOpen = f_open(&newFile, SDCardFile, FA_OPEN_APPEND | FA_OPEN_ALWAYS | FA_READ | FA_WRITE);

        if(resOpen == FR_OK) 
        {
            if (f_size(&newFile) == 0) // Write from start
            { 
                strcpy(wrData, ">>>>>>>>>>>>>>>>>>> Created new file on in SD card <<<<<<<<<<<<<\n");
                f_write(&newFile, wrData, sizeof(wrData), &bw);  
                memset(wrData, 0, sizeof(wrData));
                strcpy(wrData, "S.No, Name, Marks\n");
                f_write(&newFile, wrData, sizeof(wrData), &bw);  

                memset(wrData, 0, sizeof(wrData));
                sprintf(wrData, "%d, %4.4f,%lf\n",intVal, fVal, dVal);
                f_write(&newFile, wrData, sizeof(wrData), &bw);                      

    
                memset(wrData, 0, sizeof(wrData));
                sprintf(wrData, "%d, %4.4f,%lf\n",intVal+10, fVal+10, dVal+10);
                f_write(&newFile, wrData, sizeof(wrData), &bw);                      

                memset(wrData, 0, sizeof(wrData));
                sprintf(wrData, "%d, %4.4f,%lf\n",intVal+20, fVal+20, dVal+20);
                f_write(&newFile, wrData, sizeof(wrData), &bw);                             
            }
            else
            {
                memset(wrData, 0, sizeof(wrData));
                sprintf(wrData, "%d, %4.4f,%lf\n",intVal++, fVal++, dVal++);
                f_write(&newFile, wrData, sizeof(wrData), &bw); 
            }
            
            f_close(&newFile); /* Close the file */     
        }
    }
}

void TestSDCardRead(char *SDCardFile)
{       
    memset(wrData, 0, sizeof(wrData));
    resOpen = f_open(&newFile, SDCardFile, FA_OPEN_APPEND | FA_OPEN_ALWAYS | FA_READ);
    if(resOpen == FR_OK) 
    {        
        f_read(&newFile, wrData, sizeof(wrData), &bw);  
        f_close(&newFile); /* Close the file */ 
    }
    TestSDCardWrite(SDCardFile2);
    
    memset(wrData, 0, sizeof(wrData));
    
    resOpen = f_open(&newFile, SDCardFile2, FA_OPEN_APPEND | FA_OPEN_ALWAYS | FA_READ);
    if(resOpen == FR_OK) 
    {        
        f_read(&newFile, wrData, sizeof(wrData)-100, &bw);  
        f_close(&newFile); /* Close the file */ 
    }    
    TestSDCardWrite(SDCardFile3);
}

void TestSDCardRead1(char *SDCardFile)
{       
    memset(wrData, 0, sizeof(wrData));
    
    resOpen = f_open(&newFile, SDCardFile, FA_OPEN_ALWAYS | FA_READ);
    
    if(resOpen == FR_OK) 
    {        
        f_read(&newFile, wrData, sizeof(wrData), &bw);  
        f_close(&newFile); /* Close the file */ 
    }
}

#define READ_ROW_LENGTH     50  
i08 csvFileLastRowData[READ_ROW_LENGTH];
i08 csvLastRowOnly[READ_ROW_LENGTH];

char TestReadLastRowOfCsvFile(const i08 *filename) 
{
    FIL file;            // File object
    FRESULT result;
    UINT bytesRead;
    u32 numBytes;
    
    numBytes = READ_ROW_LENGTH;

    // Open the file
    result = f_open(&file, filename, FA_READ);
    if (result != FR_OK) {
        // Handle file open error
        f_mount(NULL, "", 0);  // Unmount the file system
        return false;
    }

    if (f_size(&file) != 0)
    {
        long unsigned int filesize = f_size(&file);
        
        // Move to the end of the file
        //result = f_lseek(&file, f_size(&file) - numBytes);
        int readBytes = filesize - numBytes;
        result = f_lseek(&file, 100);
        
        if (result != FR_OK) 
        {
            // Handle seek error
            f_close(&file);  // Close the file
            f_mount(NULL, "", 0);  // Unmount the file system
            return false;
        }

        // Read the last N bytes into the csvFileLastRowData
        //result = f_read(&file, csvFileLastRowData, numBytes, &bytesRead);
        result = f_read(&file, wrData, numBytes, &bytesRead);
        
        if (result != FR_OK) 
        {
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

char testflag = 0;
void TestSDCardOps(void)
{ 
    TestSDCardWrite(SDCardFile1);
    TestSDCardRead(SDCardFile1);
    //TestSDCardAppend();
    TestReadLastRowOfCsvFile(SDCardFile1);
    testflag = 1;
}
void TestSDCardOps1(void)
{
    TestSDCardWrite(SDCardFile1);
    TestSDCardRead1(SDCardFile1);
    //TestReadLastRowOfCsvFile(SDCardFile1);
}

int main(void)
{    
    InitExpSDCIOports(); 
    
    while(1)
    {
        for(int delay=0; delay<1000; delay++);
        TestSDCardOps1();
    }
}
