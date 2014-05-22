/* MSCAN module functions */

#include <mc9s12c32.h>
#include "mscan.h"

byte rxdata[8] = {0}; // Max of 8 bytes of data per frame

/*
 * Calculating CAN bus bit rate:
 *  b = f/KN
 * where b = bit rate
 *       f = CAN clock soruce frequency
 *       K = prescaler value
 *       N = number of quanta per bit time (1 + time_seg1 + time_seg2)
 */

/* Initialize CAN bus */
/* Only call this once during startup! */
void CANinit(void) {
    CANCTL1 |= CANCTL1_CANE_MASK;   // Enable the MSCAN module (write once)
    
    MSCAN_ENTER_INIT;               // Enter MSCAN initialization mode
    while(!MSCAN_INIT_MODE_ACTIVE); // Wait for init mode ack
    
    SET_MSCAN_CLK_SRC(MSCAN_CLK_SRC_BUS);   // Use bus clock (8MHz) for MSCAN clock source
    SET_MSCAN_PRESCALE(MSCAN_PRESCALE_8);   // Set baud rate prescaler value to 8
    SET_MSCAN_JUMP_WIDTH(MSCAN_SJW_4TQ);    // Set sync jump width to the largest possible value (faster resync)
    SET_MSCAN_SAMPLE_BITS(MSCAN_SAMPLE_1);  // Sample once per bit (vs 3 times per bit)
    
    /*
     * NOTE: Number of time quantas (Tq's) allowed one bit time is 8-25
     *  Time segment 1 can be 4-16 Tq's
     *  Time segment 2 can be 2-8 Tq's
     *  1 + time_seg1 + time_seg2 = 1 bit time
     */
    SET_MSCAN_TIME_SEG1(MSCAN_TIME_SEG1_4);
    SET_MSCAN_TIME_SEG2(MSCAN_TIME_SEG2_3);
    
    MSCAN_TIME_ENABLE;          // Enable 16-bit timestamp to messages
    MSCAN_LISTEN_MODE_DISABLE;  // Cannot be in listen mode if we want to send messages
    MSCAN_LOOPBACK_ENABLE;      // Enable loopback for testing
    
    MSCAN_EXIT_INIT;                // Exit init mode before writing to the following registers
    while(MSCAN_INIT_MODE_ACTIVE);  // Wait for init mode ack
    
    
    MSCAN_RECEIVE_INT_ENABLE;   // Enable MSCAN receiver interrupt when a new valid message is received
}

/* MSCAN transmit message */
/* non-zero return value indicates that it could not send a message */
byte CANsend(dword id, byte priority, byte length, byte *data) {
    byte txbuffer, i;
    
    // Check if all three tx buffers are already full (can't send message)
    if(!(CANTFLG & CANTFLG_TXE_MASK)) return 1;
    
    CANTBSEL = CANTFLG;     // Select lowest tx buffer using tx buffer empty flags
    txbuffer = CANTBSEL;    // Save selected tx buffer to clear flag after message is constructed
    
    *( (dword*)( (dword)(&CANTXIDR0) ) ) = id;   // Load in ID of message
    
    // Copy payload data to data segment registers (memory mapped in sequential order)
    for(i=0; i<length; i++)
        *(&CANTXDSR0 + i) = data[i];
    
    // Truncate length to 0-8 bytes
    // Most CAN controllers will assume 8 bytes of data if message length value is >8
    if(length > 8) length = 8;
    
    CANTXDLR = length;
    CANTXTBPR = priority;
    
    CANTFLG = txbuffer;     // Release tx buffer for transmission by clearing the associated flag
    while((CANTFLG & txbuffer) != txbuffer);    // Wait for transmission to complete
    
    return 0;
}

/*****************************************************************************/

/* MSCAN receiver interrupt */
// currently just writes data into a buffer and overwrites it each time there is a new message
interrupt VectorNumber_Vcanrx void CANreceiveISR(void) {
    byte length, i;
    //byte rxdata[8]; // Max of 8 bytes of data per frame - changed to be global
    
    length = ( CANRXDLR & CANRXDLR_DLC_MASK );
    
    // Copy payload data to local buffer (data segment registers memory mapped in sequential order)
    for(i=0; i<length; i++)
        rxdata[i] = *(&CANRXDSR0 + i);
    
    CANRFLG = CANRFLG_RXF_MASK; // Clear RXF flag to release rx buffer
}