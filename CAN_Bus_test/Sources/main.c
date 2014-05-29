/*
 * CAN Bus test
 *
 */

#include <hidef.h>
#include "derivative.h"
#include "timer.h"  // for msleep()
#include "mscan.h"
#include "lcd.h"    // LCD macros

#define MSCAN_CTL_ID    0x0001
#define MSCAN_CAR_ID    0x0002
#define MSCAN_FL1_ID    0x0004
#define MSCAN_FL2_ID    0x0008
#define MSCAN_FL3_ID    0x0010

#define MSCAN_NODE_ID   MSCAN_CTL_ID    // Local node ID

#define LED1    PTS_PTS2
#define LED2    PTS_PTS3
#define SW1     !PTJ_PTJ6
#define SW2     !PTJ_PTJ7

void main(void) {
    byte ret;
    byte sw1_pressed = 0, sw2_pressed = 0;
    byte data[8];
    
    // Message 1 for controller
    word id1 = MSCAN_CTL_ID;
    byte priority1 = 0x01;
    byte data1[] = "MSG1 ";
    
    // Message 2 for car
    word id2 = MSCAN_CAR_ID;
    byte priority2 = 0x01;
    byte data2[] = "MSG2 ";
    
    
    timer_init();
    CANinit(MSCAN_NODE_ID);
    
    // Clear all MSCAN receiver flags (by setting bits)
    CANRFLG = (CANRFLG_RXF_MASK | CANRFLG_OVRIF_MASK | CANRFLG_CSCIF_MASK | CANRFLG_WUPIF_MASK);
    
    DDRS_DDRS2 = 1; DDRS_DDRS3 = 1; // Enable LEDs
    DDRJ_DDRJ6 = 0; DDRJ_DDRJ7 = 0; // Enable switches
    
    msleep(16); // wait 16ms before init LCD
    LCDinit();  // initialize LCD, cursor should be visable with blink after
    
    EnableInterrupts;
    
    for(;;) {
        if(SW1 && !sw1_pressed) {
            sw1_pressed = 1;
            LED1 = 1;
            
            ret = CANsend(id1, priority1, sizeof(data1)-1, data1);
            if(ret) {
                // Message could not be sent!
            }
            
            data1[4]++;
        }
        if(SW2 && !sw2_pressed) {
            sw2_pressed = 1;
            LED2 = 1;
            
            ret = CANsend(id2, priority2, sizeof(data2)-1, data2);
            if(ret) {
                // Message could not be sent!
            }
            
            data2[4]++;
        }
        if(!SW1) {
            sw1_pressed = 0;
            LED1 = 0;
        }
        if(!SW2) {
            sw2_pressed = 0;
            LED2 = 0;
        }
        
        
        CANget(data);
        LCDhome();
        LCDprintf("%s", data);
    }
}
