/*
 * dac_max553x.h
 * MAX553X 12-bit DAC macros and functions
 * Compatible with MAX5532, MAX5533, MAX5534, MAX5535
 */
#ifndef _DAC_H
#define _DAC_H

#include <mc9s12c32.h>

#define MAX_12BIT   4096

// Slave select on Port M 3
#define SS_LO           (PTM_PTM3 = 0)
#define SS_HI           (PTM_PTM3 = 1)

// Leading control bit patterns
#define DAC_LOAD_A      0x10    // Load input register A from shift register
#define DAC_LOAD_B      0x20    // Load input register B from shift register
#define DAC_LOAD_DAC_AB 0x80    // Load DAC registers A and B from respective input registers; DAC outputs A and B updated
#define DAC_LOAD_AB     0x90    // Load input register A and DAC register A from shift register; DAC output A updated
                                // Load DAC register B from input register B; DAC output B updated
#define DAC_LOAD_BA     0xA0    // Load input register B and DAC register B from shift register; DAC output B updated
                                // Load DAC register A from input register A; DAC output A updated
#define DAC_STANDBY     0xC0    // MAX5533/MAX5535 enter standby; MAX5532/MAX5534 enter shutdown
#define DAC_NORMAL      0xD0    // Enter normal operation; DAC outputs reflect existing content in DAC registers
#define DAC_SHUTDOWN    0xE0    // Enter shutdown; DAC outputs set to high impedance
#define DAC_LOAD_ALL    0xF0    // Load input registers A and B and DAC registers A and B from shift register; DAC outputs A and B updated

// Output reference voltages (MAX5533/MAX5535)
// Used as data bits in conjunction with control bits 0xC0, 0xD0 and 0xE0
#define DAC_VREF1       0x00    // 1.214V
#define DAC_VREF2       0x40    // 1.940V
#define DAC_VREF3       0x80    // 2.425V
#define DAC_VREF4       0xC0    // 3.885V

#define DAC_VREF        DAC_VREF1   // Currently selected output reference voltage


void DACinit(void);
void DACwake(void);
void DACstandby(void);
void DACshutdown(void);
void DACdata(unsigned int data);
void DACpreload(unsigned int data);
void DACpreloadA(unsigned int data);
void DACpreloadB(unsigned int data);
void DACupdate(unsigned int data);
static void DACcmd(unsigned char cmd, unsigned int data);

#endif // _DAC_H
