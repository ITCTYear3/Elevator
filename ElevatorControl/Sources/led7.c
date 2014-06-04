/* 7 Segment Display */

#include <mc9s12c32.h>
#include "led7.h"

byte led7_table[16] = {
    // afbgcde
    0b00001000,
    0b01101011,
    0b00100100,
    0b00100001,
    0b01000011,
    0b00010001,
    0b00010000,
    0b00101011,
    0b00000000,
    0b00000011,
    0b00000010,
    0b01010000,
    0b00011100,
    0b01100000,
    0b00010100,
    0b00010110
}; 

byte led7_bars[3] = {
    // afbgcde
    0b10111111,
    0b11110111,
    0b11111101
}; 

void led7_init() {
    DDRP_DDRP1 = 1;
    DDRP_DDRP2 = 1;
    
    PTP_PTP1 = 0;  
    PTP_PTP2 = 0;
}

void led7_write(byte pattern) {
    byte i;
    for(i=0; i<8; ++i) {
        PTP_PTP1 = (pattern & 0x01);
        pattern >>= 1;
        PTP_PTP2 = 1;
        PTP_PTP2 = 0;
    }
}
