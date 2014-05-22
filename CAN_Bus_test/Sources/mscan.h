/* MSCAN module macros */
#ifndef _MSCAN_H
#define _MSCAN_H

#include "utils.h"


// MSCAN module initialization mode
// request to enter/exit initialization mode
#define MSCAN_ENTER_INIT            SET_BITS(CANCTL0,CANCTL0_INITRQ_MASK)
#define MSCAN_EXIT_INIT             CLR_BITS(CANCTL0,CANCTL0_INITRQ_MASK)
#define MSCAN_INIT_MODE_ACTIVE      ( CANCTL1 & CANCTL1_INITAK_MASK )

// MSCAN module sleep mode
// request to enter/exit sleep mode
#define MSCAN_SLEEP_MODE_ENABLE     SET_BITS(CANCTL0,CANCTL0_SLPRQ_MASK)
#define MSCAN_SLEEP_MODE_DISABLE    CLR_BITS(CANCTL0,CANCTL0_SLPRQ_MASK)
#define MSCAN_SLEEP_MODE_ACTIVE     ( CANCTL1 & CANCTL1_SLPAK_MASK )

// MSCAN internal timer (timestamps)
// free-running internal timer will assign a 16-bit timestamp to all messages
#define MSCAN_TIME_ENABLE           SET_BITS(CANCTL0,CANCTL0_TIME_MASK)
#define MSCAN_TIME_DISABLE          CLR_BITS(CANCTL0,CANCTL0_TIME_MASK)

// MSCAN module wake-up mode
// MSCAN module will restart from sleep mode when traffic detected on CAN bus (CANCTL0_SLPRQ flag gets cleared)
#define MSCAN_WAKE_MODE_ENABLE      SET_BITS(CANCTL0,CANCTL0_WUPE_MASK)
#define MSCAN_WAKE_MODE_DISABLE     CLR_BITS(CANCTL0,CANCTL0_WUPE_MASK)

// MSCAN module loopback mode
#define MSCAN_LOOPBACK_ENABLE       SET_BITS(CANCTL1,CANCTL1_LOOPB_MASK)
#define MSCAN_LOOPBACK_DISABLE      CLR_BITS(CANCTL1,CANCTL1_LOOPB_MASK)

// MSCAN module listen mode
// able to receive messages of matching ID, but no messages are transmitted back
#define MSCAN_LISTEN_MODE_ENABLE    SET_BITS(CANCTL1,CANCTL1_LISTEN_MASK)
#define MSCAN_LISTEN_MODE_DISABLE   CLR_BITS(CANCTL1,CANCTL1_LISTEN_MASK)

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

// Interrupt events
#define MSCAN_WAKE_INT_ENABLE       SET_BITS(CANRIER,CANRIER_WUPIE_MASK)    // Wake-up event interrupt
#define MSCAN_WAKE_INT_DISABLE      CLR_BITS(CANRIER,CANRIER_WUPIE_MASK)
#define MSCAN_CAN_STAT_INT_ENABLE   SET_BITS(CANRIER,CANRIER_CSCIE_MASK)    // CAN status change event interrupt
#define MSCAN_CAN_STAT_INT_DISABLE  CLR_BITS(CANRIER,CANRIER_CSCIE_MASK)
#define MSCAN_RSTATE_INT_ENABLE     SET_BITS(CANRIER,
#define MSCAN_RSTATE_INT_DISABLE    CLR_BITS(CANRIER,
#define MSCAN_TSTATE_INT_ENABLE     SET_BITS(CANRIER,
#define MSCAN_TSTATE_INT_DISABLE    CLR_BITS(CANRIER,
#define MSCAN_OVERRUN_INT_ENABLE    SET_BITS(CANRIER,CANRIER_OVRIE_MASK)    // Overrun event interrupt
#define MSCAN_OVERRUN_INT_DISABLE   CLR_BITS(CANRIER,CANRIER_OVRIE_MASK)
#define MSCAN_RECEIVE_INT_ENABLE    SET_BITS(CANRIER,CANRIER_RXFIE_MASK)    // Valid message in receiver interrupt
#define MSCAN_RECEIVE_INT_DISABLE   CLR_BITS(CANRIER,CANRIER_RXFIE_MASK)
/*
#define MSCAN_TRANSMIT0_INT_ENABLE  SET_BITTS(CANTIER,CANTIER_TXEIE0_MASK)  // Transmit buffer 0 is empty interrupt (available for transmission)
#define MSCAN_TRANSMIT0_INT_DISABLE CLR_BITTS(CANTIER,CANTIER_TXEIE0_MASK)
#define MSCAN_TRANSMIT1_INT_ENABLE  SET_BITTS(CANTIER,CANTIER_TXEIE1_MASK)  // Transmit buffer 1 is empty interrupt (available for transmission)
#define MSCAN_TRANSMIT1_INT_DISABLE CLR_BITTS(CANTIER,CANTIER_TXEIE1_MASK)
#define MSCAN_TRANSMIT2_INT_ENABLE  SET_BITTS(CANTIER,CANTIER_TXEIE2_MASK)  // Transmit buffer 2 is empty interrupt (available for transmission)
#define MSCAN_TRANSMIT2_INT_DISABLE CLR_BITTS(CANTIER,CANTIER_TXEIE2_MASK)
*/

// Abort requests
// request to abort transmit of message
#define MSCAN_ABORT_QUEUE0      SET_BITS(CANTARQ,CANTARQ_ABTRQ0_MASK)
#define MSCAN_ABORT_QUEUE1      SET_BITS(CANTARQ,CANTARQ_ABTRQ1_MASK)
#define MSCAN_ABORT_QUEUE2      SET_BITS(CANTARQ,CANTARQ_ABTRQ2_MASK)

// Abort flags
#define MSCAN_ABORT_ACK0        ( CANTAAK & CANTAAK_ABTAK0_MASK )   // Set if message in transmit buffer 0 was successfully aborted
#define MSCAN_ABORT_ACK1        ( CANTAAK & CANTAAK_ABTAK1_MASK )   // Set if message in transmit buffer 1 was successfully aborted
#define MSCAN_ABORT_ACK2        ( CANTAAK & CANTAAK_ABTAK2_MASK )   // Set if message in transmit buffer 2 was successfully aborted

// MSCAN status flags
#define MSCAN_RECEIVE_VALID     ( CANCTL0 & CANCTL0_RXFRM_MASK )    // Set if MSCAN module has received a valid message correctly (flag not valid in loopback mode)
#define MSCAN_RECEIVE_ACTIVE    ( CANCTL0 & CANCTL0_RXACT_MASK )    // Set if MSCAN module is receiving a message
#define MSCAN_SYNCED            ( CANCTL0 & CANCTL0_SYNCH_MASK )    // Set if MSCAN module is synchronized to the CAN bus

#define MSCAN_WAKE_ACTIVITY     ( CANRFLG & CANRFLG_WUPIF_MASK )    // Set if MSCAN detects bus activity while in sleep mode (CANCTL0_WUPE bit must be set)
#define MSCAN_CAN_STAT_CHANGED  ( CANRFLG & CANRFLG_CSCIF_MASK )    // Set if MSCAN changed current bus status

#define MSCAN_OVERRUN_DETECTED  ( CANRFLG & CANRFLG_OVRIF_MASK )    // Set if a data overrun is detected
//#define MSCAN_RECEIVE_FULL      ( CANRFLG & CANRFLG_RXF_MASK )      // Set if a valid message is available at the receiver buffer, must be cleared to release the buffer

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


/*****************************************************************************/

void CANinit(void);
byte CANsend(dword, byte, byte, byte*);


#endif // _MSCAN_H