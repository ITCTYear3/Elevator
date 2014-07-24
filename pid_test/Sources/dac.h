/* MAX551X 8-bit DAC macros and functions */
/*   MAX5512, MAX5513, MAX5514, MAX5515   */
#ifndef _DAC_H
#define _DAC_H

#include <mc9s12c32.h>
#include "spi.h"

#define LO_NIBBLE(value)    ((value) & 0x0F)        // Get lower nibble of byte
#define HI_NIBBLE(value)    (((value) & 0xF0) >> 4) // Get upper nibble of byte

#define SS_LO           (PTM_PTM3 = 0)
#define SS_HI           (PTM_PTM3 = 1)

#define DAC_SUB_BITS    0x00    // Trailing four sub-bits always set to zero

// Leading control bit patterns
#define DAC_LOAD_A      0x10    // Load input register A from shift register
#define DAC_LOAD_B      0x20    // Load input register B from shift register
#define DAC_LOAD_DAC_AB 0x80    // Load DAC registers A and B from respective input registers; DAC outputs A and B updated
#define DAC_LOAD_AB     0x90    // Load input register A and DAC register A from shift register; DAC output A updated
                                // Load DAC register B from input register B; DAC output B updated
#define DAC_LOAD_BA     0xA0    // Load input register B and DAC register B from shift register; DAC output B updated
                                // Load DAC register A from input register A; DAC output A updated
#define DAC_STANDBY     0xC0    // MAX5513/MAX5515 enter standby; MAX5512/MAX5514 enter shutdown
#define DAC_NORMAL      0xD0    // Enter normal operation; DAC outputs reflect existing content in DAC registers
#define DAC_SHUTDOWN    0xE0    // Enter shutdown; DAC outputs set to high impedance
#define DAC_LOAD_ALL    0xF0    // Load input registers A and B and DAC registers A and B from shift register; DAC outputs A and B updated

// Output reference voltages (MAX5513/MAX5515)
// Used as data bits in conjunction with control bits 0xC0, 0xD0 and 0xE0
#define DAC_VREF1       0x00    // 1.214V
#define DAC_VREF2       0x40    // 1.940V
#define DAC_VREF3       0x80    // 2.425V
#define DAC_VREF4       0xC0    // 3.885V

#define DAC_VREF        DAC_VREF3   // Currently selected output reference voltage


void DACinit(void);
void DACwake(void);
void DACstandby(void);
void DACshutdown(void);
void DACdata(unsigned char);
void DACpreload(unsigned char);
void DACpreloadA(unsigned char);
void DACpreloadB(unsigned char);
void DACupdate(unsigned char);
static void DACcmd(unsigned char, unsigned char);

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
    high_byte = (cmd & 0xF0) | HI_NIBBLE(data);
    low_byte = (LO_NIBBLE(data) << 4) | (DAC_SUB_BITS & 0x0F);
    
    // Send 16-bit command composed of two 8-bit transfers consecutively
    SS_LO;  // SS pulled low
    SPIputc(high_byte);
    SPIputc(low_byte);
    SS_HI;  // SS pulled high
}

#endif // _DAC_H