/* 
 * Serial/CANbus link 
 */

#ifndef _SERIALCAN_H
#define _SERIALCAN_H

#include "mscan.h"

char readSerialCANframe(CANframe *frame);
void sendSerialCANframe(CANframe *frame);
void runSerialCAN(word id);

#endif