/*
 * dac_max551x.c
 * MAX551X 8-bit DAC macros and functions
 * Compatible with MAX5512, MAX5513, MAX5514, MAX5515
 */

#include <mc9s12c32.h>
#include "dac_max551x.h"
#include "spi.h"


/* Initialize DAC */
void DACinit(void) {
    DACwake();  // Wake up DAC
}

/* Wake up DAC from shutdown or standby */
void DACwake(void) {
    DACcmd(DAC_NORMAL, DAC_VREF);
    
    // Disable slave select with master mode
    SPICR2_MODFEN = 0;
    SPICR1_SSOE = 0;
    
    DDRM_DDRM3 = 1; // Set SS pin as output
}

/* Put DAC into standby */
void DACstandby(void) {
    DACcmd(DAC_STANDBY, DAC_VREF);
}

/* Put DAC into shutdown */
void DACshutdown(void) {
    DACcmd(DAC_SHUTDOWN, DAC_VREF);
}

/* Write data to DAC and update immediately */
void DACdata(unsigned char data) {
    DACcmd(DAC_LOAD_ALL, data);
}

/* Preload data into DAC input registers A and B */
void DACpreload(unsigned char data) {
    DACcmd(DAC_LOAD_A, data);
    DACcmd(DAC_LOAD_B, data);
}

/* Preload data into DAC input register A */
void DACpreloadA(unsigned char data) {
    DACcmd(DAC_LOAD_A, data);
}

/* Preload data into DAC input register B */
void DACpreloadB(unsigned char data) {
    DACcmd(DAC_LOAD_B, data);
}

/* Update DAC channels A and B from DAC registers A and B */
void DACupdate(unsigned char data) {
    DACcmd(DAC_LOAD_DAC_AB, data);
}


/* Send DAC command */
void DACcmd(unsigned char cmd, unsigned char data) {
    char high_byte, low_byte;
    
    // Assemble upper and lower byte
    high_byte = (cmd & 0xF0) | ((data & 0xF0) >> 4);
    low_byte = ((data & 0x0F) << 4) | (DAC_SUB_BITS & 0x0F);
    
    // Send 16-bit command composed of two 8-bit transfers consecutively
    SS_LO;  // SS pulled low
    SPIputc(high_byte);
    SPIputc(low_byte);
    SS_HI;  // SS pulled high
}
