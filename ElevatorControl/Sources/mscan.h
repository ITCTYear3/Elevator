/* MSCAN module macros */
#ifndef _MSCAN_H
#define _MSCAN_H

#include "utils.h"

#define PAYLOAD_SIZE    8   // Max of 8 bytes of data per CAN frame

// MSCAN module clock source
// from either the bus clock or the oscillator clock
#define SET_MSCAN_CLK_SRC(source)   SET_BITS(CANCTL1,(source) << CANCTL1_CLKSRC_MASK)
#define MSCAN_CLK_SRC_BUS           0x01    // 8MHz clock
#define MSCAN_CLK_SRC_OSC           0x00    // 16MHz clock

// Synchronization jump width if out of sync
// 1-4 Tq's of jump width
#define SET_MSCAN_JUMP_WIDTH(width) FORCE_BITS(CANBTR0,CANBTR0_SJW_MASK,(width) << CANBTR0_SJW_BITNUM)
#define MSCAN_SJW_1TQ   0x00    // Jump width of 1 Tq clock cycle
#define MSCAN_SJW_2TQ   0x01    // Jump width of 2 Tq clock cycles
#define MSCAN_SJW_3TQ   0x02    // Jump width of 3 Tq clock cycles
#define MSCAN_SJW_4TQ   0x03    // Jump width of 4 Tq clock cycles

// MSCAN baud rate prescaler
// 1-64 prescale value, actual value written to register is prescale value subtract one
#define SET_MSCAN_PRESCALE(scale)   FORCE_BITS(CANBTR0,CANBTR0_BRP_MASK,(scale) << CANBTR0_BRP_BITNUM)
#define MSCAN_PRESCALE_1        0x00
#define MSCAN_PRESCALE_2        0x01
#define MSCAN_PRESCALE_3        0x02
#define MSCAN_PRESCALE_4        0x03
#define MSCAN_PRESCALE_5        0x04
#define MSCAN_PRESCALE_6        0x05
#define MSCAN_PRESCALE_7        0x06
#define MSCAN_PRESCALE_8        0x07
#define MSCAN_PRESCALE_9        0x08
#define MSCAN_PRESCALE_10       0x09
#define MSCAN_PRESCALE_16       0x0F
#define MSCAN_PRESCALE_32       0x1F
#define MSCAN_PRESCALE_64       0x3F

// Number of samples per bit
#define SET_MSCAN_SAMPLE_BITS(bits) FORCE_BITS(CANBTR1,CANBTR1_SAMP_MASK,(bits) << 7U)
#define MSCAN_SAMPLE_1          0x00    // one sample per bit
#define MSCAN_SAMPLE_3          0x01    // three samples per bit

// Time segment 1
// can be 4-16 Tq clock cycles long
#define SET_MSCAN_TIME_SEG1(cycles) FORCE_BITS(CANBTR1,CANBTR1_TSEG_10_MASK,(cycles) << CANBTR1_TSEG_10_BITNUM)
#define MSCAN_TIME_SEG1_4       0x03
#define MSCAN_TIME_SEG1_5       0x04
#define MSCAN_TIME_SEG1_6       0x05
#define MSCAN_TIME_SEG1_7       0x06
#define MSCAN_TIME_SEG1_8       0x07
#define MSCAN_TIME_SEG1_9       0x08
#define MSCAN_TIME_SEG1_10      0x09
#define MSCAN_TIME_SEG1_11      0x0A
#define MSCAN_TIME_SEG1_12      0x0B
#define MSCAN_TIME_SEG1_13      0x0C
#define MSCAN_TIME_SEG1_14      0x0D
#define MSCAN_TIME_SEG1_15      0x0E
#define MSCAN_TIME_SEG1_16      0x0F

// Time segment 2
// can be 2-8 Tq clock cycles long
#define SET_MSCAN_TIME_SEG2(cycles) FORCE_BITS(CANBTR1,CANBTR1_TSEG_20_MASK,(cycles) << CANBTR1_TSEG_20_BITNUM)
#define MSCAN_TIME_SEG2_2       0x01
#define MSCAN_TIME_SEG2_3       0x02
#define MSCAN_TIME_SEG2_4       0x03
#define MSCAN_TIME_SEG2_5       0x04
#define MSCAN_TIME_SEG2_6       0x05
#define MSCAN_TIME_SEG2_7       0x06
#define MSCAN_TIME_SEG2_8       0x07

// MSCAN status flags
#define MSCAN_RECEIVE_VALID     CANCTL0_RXFRM   // Set if MSCAN module has received a valid message correctly (flag not valid in loopback mode)
#define MSCAN_RECEIVE_ACTIVE    CANCTL0_RXACT   // Set if MSCAN module is receiving a message
#define MSCAN_SYNCED            CANCTL0_SYNCH   // Set if MSCAN module is synchronized to the CAN bus
#define MSCAN_WAKE_ACTIVITY     CANRFLG_WUPIF   // Set if MSCAN detects bus activity while in sleep mode (CANCTL0_WUPE bit must be set)
#define MSCAN_CAN_STAT_CHANGED  CANRFLG_CSCIF   // Set if MSCAN changed current bus status
#define MSCAN_OVERRUN_DETECTED  CANRFLG_OVRIF   // Set if a data overrun is detected

/*
    Receiver Bus Status (CANRFLG_CSCIF bit must be set)
    0x00 = RxOK:     0 <= Receive error counter <= 96
    0x01 = RxWRN:   96 <= Receive error counter <= 127
    0x02 = RxERR:  127 <= Receive error counter <= 255
    0x03 = Bus Off:       Transmit error counter > 255
*/
#define MSCAN_RSTAT_ERRORS      ((CANRFLG & CANRFLG_RSTAT_MASK) >> CANRFLG_RSTAT_BITNUM )

/*
    Transmitter Bus Status
    0x00 = TxOK:     0 <= Transmit error counter <= 96
    0x01 = TxWRN:   96 <= Transmit error counter <= 127
    0x02 = TxERR:  127 <= Transmit error counter <= 255
    0x03 = Bus Off:       Transmit error counter >  255
*/
#define MSCAN_TSTAT_ERRORS      ((CANRFLG & CANRFLG_TSTAT_MASK) >> CANRFLG_TSTAT_BITNUM )

// MSCAN acceptance filter modes
#define MSCAN_ACC_MODE(mode)    FORCE_BITS(CANIDAC,CANIDAC_IDAM_MASK,(mode) << CANIDAC_IDAM_BITNUM)
#define MSCAN_ACC_2_32      0x00    // Two 32bit acceptance filters
#define MSCAN_ACC_4_16      0x01    // Four 16bit acceptance filters
#define MSCAN_ACC_8_8       0x02    // Eight 8bit acceptance filters
#define MSCAN_ACC_CLOSED    0x03    // Filter closed; no message is copied into receive foreground buffer, RXF flag is never set


// CAN frame structure
typedef struct {
    word id;    // 11bits usable for CAN frame ID
    byte priority;
    byte length;
    byte payload[PAYLOAD_SIZE]; // First byte in payload determines what the remaining bytes signify
} CANframe;


void CANinit(word id);
byte CANsend(CANframe *frame);
void CANget(byte *data);
byte data_available(void);
void CANput(byte *data);
byte data_sent(void);
CANframe *last_txframe(void);	  
byte data_received(void);
CANframe *last_rxframe(void);

#endif // _MSCAN_H