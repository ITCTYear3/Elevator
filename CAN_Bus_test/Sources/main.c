/*
 * Elevator Control System
 *
 * Chris Brown & Jack Morgan
 * ITCT Semester 6
 */

#include <hidef.h>
#include "derivative.h"
#include "timer.h"      // msleep()
#include "mscan.h"
#include "lcd.h"
#include "protocol.h"   // CAN node IDs and payload data IDs

#define LED1    PTS_PTS2
#define LED2    PTS_PTS3
#define SW1     !PTJ_PTJ6
#define SW2     !PTJ_PTJ7

void callbox(void);


void main(void) {
    
    DDRS_DDRS2 = 1; DDRS_DDRS3 = 1; // Enable LEDs
    DDRJ_DDRJ6 = 0; DDRJ_DDRJ7 = 0; // Enable switches
    
    timer_init();
    CANinit(MSCAN_NODE_ID);
    
    // Clear all MSCAN receiver flags (by setting bits)
    CANRFLG = (CANRFLG_RXF_MASK | CANRFLG_OVRIF_MASK | CANRFLG_CSCIF_MASK | CANRFLG_WUPIF_MASK);
    
    msleep(16); // wait 16ms before init LCD
    LCDinit();  // initialize LCD, cursor should be visible with blink after
    
    EnableInterrupts;
    
    for(;;) {
        callbox();
    }
}

/*
 * Callbox functionality
 * Watch for button presses, and accept elevator location messages
 */
void callbox(void) {
    byte ret;
    byte sw1_pressed = 0, sw2_pressed = 0;
    char *command, *floor, *direction;
    
    CANframe txframe1, txframe2;    // Transmitted CAN frames
    byte rxmessage[PAYLOAD_SIZE];   // Received data payload
    
    // Message to floor 1 callbox; direction up
    txframe1.id = MSCAN_FL1_ID;
    txframe1.priority = 0x01;
    txframe1.length = 3;
    txframe1.payload[0] = CMD_LOCATION;
    txframe1.payload[1] = FLOOR1;
    txframe1.payload[2] = DIRECTION_UP;
    
    // Message to floor 1 callbox; direction down
    txframe2.id = MSCAN_FL1_ID;
    txframe2.priority = 0x01;
    txframe2.length = 3;
    txframe2.payload[0] = CMD_LOCATION;
    txframe2.payload[1] = FLOOR1;
    txframe2.payload[2] = DIRECTION_DOWN;
    
    
    if(SW1 && !sw1_pressed) {
        sw1_pressed = 1;
        LED1 = 1;
        
        ret = CANsend(&txframe1);
        if(ret) {
            // Message could not be sent!
        }
    }
    if(SW2 && !sw2_pressed) {
        sw2_pressed = 1;
        LED2 = 1;
        
        ret = CANsend(&txframe2);
        if(ret) {
            // Message could not be sent!
        }
    }
    if(!SW1) {
        sw1_pressed = 0;
        LED1 = 0;
    }
    if(!SW2) {
        sw2_pressed = 0;
        LED2 = 0;
    }
    
    if(data_available()) {
        
        CANget(rxmessage);
        
        switch(rxmessage[0]) {
            case CMD_LOCATION:
                command = "Loc";
                break;
            case CMD_BUTTON_CALL:
                command = "Call";
                break;
            case CMD_BUTTON_CAR:
                command = "Car";
                break;
            case CMD_DISP_APPEND:
                command = "Disp";
                break;
            case CMD_ERROR:
                command = "Err";
                break;
            default:
                // Command didn't match known commands!
                goto cmd_error;
        }
        
        switch(rxmessage[1]) {
            case FLOOR1:
                floor = "1";
                break;
            case FLOOR2:
                floor = "2";
                break;
            case FLOOR3:
                floor = "3";
                break;
            default:
                // Command didn't match known commands!
                goto cmd_error;
        }
        
        switch(rxmessage[2]) {
            case DIRECTION_UP:
                direction = "Up";
                break;
            case DIRECTION_DOWN:
                direction = "Down";
                break;
            case DIRECTION_STATIONARY:
                direction = "Static";
                break;
            default:
                // Command didn't match known commands!
                goto cmd_error;
        }
        
        LCDhome();
        LCDclear();
        LCDprintf("Command: %s\nFloor%s Dir: %s", command, floor, direction);
        
        return;
        
cmd_error:
        LCDhome();
        LCDclear();
        LCDprintf("Error in\ncommand");
    }
}
