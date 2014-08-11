/*
 * dac_max553x.c
 * MAX553X 12-bit DAC macros and functions
 * Compatible with MAX5532, MAX5533, MAX5534, MAX5535
 */

#include <mc9s12c32.h>
#include "dac_max553x.h"
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
void DACdata(unsigned int data) {
    DACcmd(DAC_LOAD_ALL, data);
}

/* Preload data into DAC input registers A and B */
void DACpreload(unsigned int data) {
    DACcmd(DAC_LOAD_A, data);
    DACcmd(DAC_LOAD_B, data);
}

/* Preload data into DAC input register A */
void DACpreloadA(unsigned int data) {
    DACcmd(DAC_LOAD_A, data);
}

/* Preload data into DAC input register B */
void DACpreloadB(unsigned int data) {
    DACcmd(DAC_LOAD_B, data);
}

/*
 * Load data into DAC input register A and update channel output
 * Update channel B output with data currently in DAC register B
 */
void DACloadAshiftB(unsigned int data) {
    DACcmd(DAC_LOAD_AB, data);
}

/*
 * Load data into DAC input register B and update channel output
 * Update channel A output with data currently in DAC register A
 */
void DACloadBshiftA(unsigned int data) {
    DACcmd(DAC_LOAD_BA, data);
}

/* Update DAC channels A and B from DAC registers A and B */
void DACupdate(unsigned int data) {
    DACcmd(DAC_LOAD_DAC_AB, data);
}


/* Send DAC command */
/* NOTE: Only the 12 least significant bits will be used as data! */
void DACcmd(unsigned char cmd, unsigned int data) {
    char high_byte, low_byte;
    
    // Assemble upper and lower byte
    high_byte = (unsigned char)((cmd & 0xF0) | ((data & 0x0F00) >> 8));
    low_byte = (unsigned char)(data & 0x00FF);
    
    // Send 16-bit command composed of two 8-bit transfers consecutively
    SS_LO;  // SS pulled low
    SPIputc(high_byte);
    SPIputc(low_byte);
    SS_HI;  // SS pulled high
}
