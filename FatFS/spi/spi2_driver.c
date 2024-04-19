/*
    (c) 2020 Microchip Technology Inc. and its subsidiaries. You may use this
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

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
//#include "mcc.h"
#include "spi2_driver.h"
#include <xc.h>

#define MAX_WAIT_COUNT      10000

void (*spi2_interruptHandler)(void); 

void spi2_close(void)
{
    SPI2STATbits.SPIEN = 0;
}

//con1 == SPIxCON1, con2 == SPIxCON2, stat == SPIxSTAT, operation == Master/Slave
typedef struct { uint16_t con1; uint16_t con2; uint16_t stat; uint8_t operation;} spi2_configuration_t;
static const spi2_configuration_t spi2_configuration[] = {   
    { 0x012F, 0x0000, 0x0000, 0 }, //SDFAST_CONFIG
    { 0x0122, 0x0000, 0x0000, 0 }, //SDSLOW_CONFIG
    { 0x0122, 0x0000, 0x0000, 0 } //spi2_DEFAULT
};

void Init_spi2_FastSpeed(void)
{
	// setup the SPI peripheral
	SPI2STAT = 0x0;  // disable the SPI module (just in case)
	
    SPI2CON1 = 0x012F;//0x0122;	// FRAMEN = 0, SPIFSD = 0, DISSDO = 0, MODE16 = 0; SMP = 0; CKP = 1; CKE = 1; SSEN = 0; MSTEN = 1; SPRE = 0b000, PPRE = 0b01
    SPI2CON2 = 0;
    
	SPI2STAT = 0x8000; // enable the SPI module
}

void Init_spi2_LowSpeed(void)
{
	// setup the SPI peripheral
	SPI2STAT = 0x0;  // disable the SPI module (just in case)
	
    SPI2CON1 = 0x0122;	// FRAMEN = 0, SPIFSD = 0, DISSDO = 0, MODE16 = 0; SMP = 0; CKP = 1; CKE = 1; SSEN = 0; MSTEN = 1; SPRE = 0b000, PPRE = 0b01
    SPI2CON2 = 0;
    
	SPI2STAT = 0x8000; // enable the SPI module
}

void Init_spi2_Default(void)
{
	// setup the SPI peripheral
	SPI2STAT = 0x0;  // disable the SPI module (just in case)
	
    SPI2CON1 = 0x0122;//0x0122;	// FRAMEN = 0, SPIFSD = 0, DISSDO = 0, MODE16 = 0; SMP = 0; CKP = 1; CKE = 1; SSEN = 0; MSTEN = 1; SPRE = 0b000, PPRE = 0b01
    SPI2CON2 = 0;
    
	SPI2STAT = 0x8000; // enable the SPI module
}

bool spi2_open(spi2_modes spiUniqueConfiguration)
{    
    switch(spiUniqueConfiguration)
    {
        case SDFAST_CONFIG:
                Init_spi2_FastSpeed();
                SPI2STATbits.SPIEN = 1;    
                return true;        
            break;

        case SDSLOW_CONFIG:
                Init_spi2_LowSpeed();
                SPI2STATbits.SPIEN = 1;    
                return true;
            break;
        
        default:
        case SPI2_DEFAULT:
                Init_spi2_Default();
                SPI2STATbits.SPIEN = 1;    
                return true;
            break; 
    }
}

uint8_t spi2_readByte(void)
{
    return SPI2BUF;
}

// Full Duplex SPI Functions
uint8_t spi2_exchangeByte(uint8_t data)
{
    SPI2BUF = data;
    while(!SPI2STATbits.SPIRBF);
    return SPI2BUF;
}

void spi2_exchangeBlock(void *block, size_t blockSize)
{
    uint8_t *data = block;
    while(blockSize--)
    {
        *data = spi2_exchangeByte(*data );
        data++;
    }
}

// Half Duplex SPI Functions
void spi2_writeBlock(void *block, size_t blockSize)
{
    uint8_t *data = block;
    while(blockSize--)
    {
        spi2_exchangeByte(*data++);
    }
}

void spi2_readBlock(void *block, size_t blockSize)
{
    uint8_t *data = block;
    while(blockSize--)
    {
        *data++ = spi2_exchangeByte(0);
    }
}

void spi2_writeByte(uint8_t byte)
{
    //SPI2BUF = byte;
    
    uint16_t waitCounter = 0;

    // Wait for any pending transmissions
    while (SPI2STATbits.SPITBF && (++waitCounter < MAX_WAIT_COUNT));

    // Word write
    if (SPI2CON1bits.MODE16)
    {
        SPI2BUF = byte;
    }
    // Byte write
    else
    {
        SPI2BUF = byte & 0xff;
    }   
}

void spi2_setSpiISR(void(*handler)(void))
{
    spi2_interruptHandler = handler;
}

void spi2_isr(void)
{
    if(IFS2bits.SPI2IF == 1){
        if(spi2_interruptHandler){
            spi2_interruptHandler();
        }
        IFS2bits.SPI2IF = 0;
    }
}
