/*
 * Elevator Control System
 *
 * Chris Brown & Jack Morgan
 * ITCT Semester 6
 */

#include <hidef.h>
#include "derivative.h"
#include "protocol.h"   // CAN node IDs and payload data IDs
#include "timer.h"      // msleep()
#include "mscan.h"
#include "lcd.h"

#define LED1    PTS_PTS2
#define LED2    PTS_PTS3
#define SW1     !PTJ_PTJ6
#define SW2     !PTJ_PTJ7

// Set local node ID (unique to each node)
#define MSCAN_NODE_ID   MSCAN_CTL_ID
//#define MSCAN_NODE_ID   MSCAN_CAR_ID
//#define MSCAN_NODE_ID   MSCAN_FL1_ID
//#define MSCAN_NODE_ID   MSCAN_FL2_ID
//#define MSCAN_NODE_ID   MSCAN_FL3_ID

#if MSCAN_NODE_ID & MSCAN_CTL_ID

    void controller(void);
    #define RUN() controller();

#elif MSCAN_NODE_ID & MSCAN_CAR_ID    

    void car(void);
    #define RUN() car();

#elif MSCAN_NODE_ID & (MSCAN_FL1_ID | MSCAN_FL2_ID | MSCAN_FL2_ID)

    void callbox(byte my_floor);
    #define RUN() callbox();

#endif


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
        RUN();
    }
}

/*
 * Controller functionality
 * - Send elevator location messages to callboxes
 * - Listen for button press messages
 */
void controller(void) {
    byte ret;
    byte sw1_pressed = 0, sw2_pressed = 0;
    char *command, *floor, *direction;
    
    CANframe txframe;               // Transmitted CAN frame
    byte rxmessage[PAYLOAD_SIZE];   // Received data payload
    
    
    if(SW1 && !sw1_pressed) {
        sw1_pressed = 1;
        LED1 = 1;
        
        // Message to floor 1 callbox; direction up
        txframe.id = MSCAN_FL1_ID;
        txframe.priority = 0x01;
        txframe.length = 3;
        txframe.payload[0] = CMD_LOCATION;
        txframe.payload[1] = FLOOR1;
        txframe.payload[2] = DIRECTION_UP;
        
        ret = CANsend(&txframe);
        if(ret) {
            // Message could not be sent!
        }
    }
    if(SW2 && !sw2_pressed) {
        sw2_pressed = 1;
        LED2 = 1;
        
        // Message to floor 1 callbox; direction down
        txframe.id = MSCAN_FL1_ID;
        txframe.priority = 0x01;
        txframe.length = 3;
        txframe.payload[0] = CMD_LOCATION;
        txframe.payload[1] = FLOOR1;
        txframe.payload[2] = DIRECTION_DOWN;
        
        ret = CANsend(&txframe);
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
            case BUTTON_UP:
                direction = "Up";
                break;
            case BUTTON_DOWN:
                direction = "Down";
                break;
            default:
                // Command didn't match known commands!
                goto cmd_error;
        }
        
        LCDclear();
        LCDprintf("Command: %s\nFloor%s Dir: %s", command, floor, direction);
        
        return;
        
cmd_error:
        LCDclear();
        LCDprintf("Error in\ncommand");
    }
}

/*
 * Callbox functionality
 * - Listen for button presses, and accept elevator location messages
 */
void callbox(byte my_floor) {
    byte ret;
    byte sw1_pressed = 0, sw2_pressed = 0;
    CANframe txframe;               // Transmitted CAN frame
    byte rxmessage[PAYLOAD_SIZE];   // Received data payload
    static byte floor, direction;
    
    floor = my_floor;                   // Assume starting floor is at this callbox floor
    direction = DIRECTION_STATIONARY;   // Assume starting car direction is stationary
    
    if(SW1 && !sw1_pressed) {
        sw1_pressed = 1;
        LED1 = 1;
        
        // Message to controller; up button pressed
        txframe.id = MSCAN_CTL_ID;
        txframe.priority = 0x01;
        txframe.length = 3;
        txframe.payload[0] = CMD_BUTTON_CALL;
        txframe.payload[1] = my_floor;
        txframe.payload[2] = BUTTON_UP;
        
        ret = CANsend(&txframe);
        if(ret) {
            // Message could not be sent!
        }
    }
    if(SW2 && !sw2_pressed) {
        sw2_pressed = 1;
        LED2 = 1;
        
        // Message to controller; down button pressed
        txframe.id = MSCAN_CTL_ID;
        txframe.priority = 0x01;
        txframe.length = 3;
        txframe.payload[0] = CMD_BUTTON_CALL;
        txframe.payload[1] = my_floor;
        txframe.payload[2] = BUTTON_DOWN;
        
        ret = CANsend(&txframe);
        if(ret) {
            // Message could not be sent!
        }
    }
    
    if(!SW1 && (floor == my_floor)) sw1_pressed = 0;
    if(!SW2 && (floor == my_floor)) sw2_pressed = 0;
    
    if(data_available()) {
        
        CANget(rxmessage);
        
        switch(rxmessage[0]) {
            case CMD_LOCATION:
                floor = rxmessage[1];
                direction = rxmessage[2];
                break;
            case CMD_ERROR:
                // Error condition received!
                break;
            default:
                // Command didn't match known commands!
                break;
        }
        
        LCDclear();
        LCDprintf("Floor: %s\nDir: %s", floor, direction);
    }
}
