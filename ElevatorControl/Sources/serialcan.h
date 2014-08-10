/* Serial <--> CANbus link */
#ifndef _SERIALCAN_H
#define _SERIALCAN_H

#include "mscan.h"

// readSerialCANframe() return values
#define RX_COMPLETE 0
#define RX_PARTIAL  1
#define RX_IDLE     2

// readSerialCANframe() state machine states
#define RX_STATE_IDH        0
#define RX_STATE_IDL        1
#define RX_STATE_PRIORITY   2
#define RX_STATE_LENGTH     3
#define RX_STATE_PAYLOAD    4
#define RX_STATE_DONE       5

byte readSerialCANframe(CANframe *frame);
static void sendSerialCANframe(CANframe *frame);
void runSerialCAN(word id);

#endif // _SERIALCAN_H