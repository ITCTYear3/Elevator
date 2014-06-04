/* 7 Segment Display */
#ifndef _LED7_H
#define _LED7_H

                    // afbgcde
#define LED7_OFF    0b01111111
#define LED7_HBARS  0b00110101

void led7_init(void);
void led7_write(byte pattern);

extern byte led7_table[16];
extern byte led7_bars[3];

#endif // _LED7_H