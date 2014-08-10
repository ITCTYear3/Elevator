/* Serial <--> CANbus link */

#include <mc9s12c32.h>
#include "serialcan.h"
#include "utils.h"
#include "mscan.h"
#include "sci.h"

static CANframe framebuf;           // Rx frame buffer for partial frames
static byte state = RX_STATE_IDH;   // State machine variable
static byte length;                 // Total number of payload bytes in the current frame

#pragma MESSAGE DISABLE C1420   // Function call result ignored warning disable (for sci_readByte)
/*
 * Reads a CAN frame from the PC monitor via serial
 * Returns RX_COMPLETE if a complete frame has been received, RX_PARTIAL if a partial frame has been received, or RX_IDLE if waiting for a frame to arrive
 * Non-blocking: partial frames are buffered so the user's frame is unmodified until a full frame is ready, 
 * and persistent between calls so data will not be lost.
 */
char readSerialCANframe(CANframe *frame) {
    byte b;
    char retCode = ( state == RX_STATE_IDH ? RX_IDLE : RX_PARTIAL );    // What to return if no bytes are available
    while ( sci_bytesAvailable() ) {
        retCode = RX_PARTIAL;
        sci_readByte(&b);   // Read in one byte at a time per state loop
        switch ( state ) {
            case RX_STATE_IDH:
                FORCE_BITS(framebuf.id, 0xFF00, ((word)b)<<8);
                state = RX_STATE_IDL;
                break;
            case RX_STATE_IDL:
                FORCE_BITS(framebuf.id, 0x00FF, b);
                state = RX_STATE_PRIORITY;
                break;
            case RX_STATE_PRIORITY:  
                framebuf.priority = b;
                state = RX_STATE_LENGTH;
                break;
            case RX_STATE_LENGTH:  
                length = b;
                framebuf.length = 0;    // Note: stores the number of received bytes, not the expected total
                state = RX_STATE_PAYLOAD;
                break;
            case RX_STATE_PAYLOAD: 
                framebuf.payload[framebuf.length++] = b;
                // Got a full frame?
                if ( framebuf.length == length ) {
                    state = RX_STATE_IDH;
                    *frame = framebuf;  // Copy to user
                    return RX_COMPLETE; // Return immediately and ignore any remaining bytes until next time
                }
                break;
            default:
                break;
        }
    }
    return retCode;
}


/*
 * Transmits a CAN frame to the PC monitor via serial
 * Blocks until finished to prevent missed bus frames
 */
void sendSerialCANframe(CANframe *frame) {
    byte i;
    byte length = sizeof(CANframe) - (PAYLOAD_SIZE - frame->length);
    byte *buf = (byte*)frame; 
    // Transmit the frame buffer
    for ( i = 0; i < length; i++ ) {
        // sendByte returns immediately if the buffer is full
        // Attempt to send each byte until successful
        while ( ! sci_sendByte(buf[i]) );
    }
}


/*
 * User-facing routine
 * Checks for new frames from PC and, and send PC any new bus frames   
 * Should be called once per main loop iteration during normal operation
 */
void runSerialCAN(word id) {
    CANframe rxFrame;
    // Check for a new frame from the PC
    if (  readSerialCANframe(&rxFrame) == RX_COMPLETE ) {
        if ( rxFrame.id == id ) {
            CANput(rxFrame.payload);
        } else {
            CANsend(&rxFrame);
        }
    }
    // Check for a recently sent frame
    if ( data_sent() ) {
        sendSerialCANframe(last_txframe());
    }
    // Check for a recently received frame
    if ( data_received() ) {
        sendSerialCANframe(last_rxframe());
    }
}
