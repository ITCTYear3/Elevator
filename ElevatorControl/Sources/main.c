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
#include "lcdspi.h"
#include "dist.h"
#include "led7.h"
#include "sci.h"
#include "mcutilib.h"

#define LED1    PTS_PTS2
#define LED2    PTS_PTS3
#define SW1     !PTJ_PTJ6
#define SW2     !PTJ_PTJ7

#define CM_PER_FLOOR 15

// Set local node ID (unique to each node)
//#define MSCAN_NODE_ID   MSCAN_CTL_ID
//#define MSCAN_NODE_ID   MSCAN_CAR_ID
#define MSCAN_NODE_ID   MSCAN_FL1_ID
//#define MSCAN_NODE_ID   MSCAN_FL2_ID
//#define MSCAN_NODE_ID   MSCAN_FL3_ID

#if MSCAN_NODE_ID == MSCAN_CTL_ID

    void controller(void);
    #define RUN()   controller();

#elif MSCAN_NODE_ID == MSCAN_CAR_ID    

    void car(void);
    #define RUN()   car();

#elif MSCAN_NODE_ID & (MSCAN_FL1_ID | MSCAN_FL2_ID | MSCAN_FL3_ID)

    void callbox(byte my_floor);
    #if MSCAN_NODE_ID == MSCAN_FL1_ID
        #define RUN()   callbox(FLOOR1);
    #elif MSCAN_NODE_ID == MSCAN_FL2_ID
        #define RUN()   callbox(FLOOR2);
    #elif MSCAN_NODE_ID == MSCAN_FL3_ID
        #define RUN()   callbox(FLOOR3);
    #endif

#endif

#pragma MESSAGE DISABLE C1420
void main(void) {
    
    byte b;
    
    DDRS_DDRS2 = 1; DDRS_DDRS3 = 1; // Enable LEDs
    DDRJ_DDRJ6 = 0; DDRJ_DDRJ7 = 0; // Enable switches
    
    timer_init();
    CANinit(MSCAN_NODE_ID);
    
    // Clear all MSCAN receiver flags (by setting bits)
    CANRFLG = (CANRFLG_RXF_MASK | CANRFLG_OVRIF_MASK | CANRFLG_CSCIF_MASK | CANRFLG_WUPIF_MASK);
    
    msleep(16); // wait 16ms before init LCD
    LCDinit();  // initialize LCD, cursor should be visible with blink after
    
    led7_init();
    led7_write(LED7_HBARS);
    
    lcd_init();
    
    sci_init();
    
    EnableInterrupts;
    
    sci_sendBytes((byte*)"Ready", 5);
    
    for(;;) {
       /*
        while ( sci_bytesAvailable() ) {
        	sci_readByte(&b);
			lcd_putc(b);  
        }   */
       RUN();
       
    }
}

/*
 * Transmits the floor that the current elevator car is on
 * to the internal panel and to each call box
 */
#pragma MESSAGE DISABLE C1420   // Result of function call warning (for CANsend() )
void update_floor(byte floor) {
    CANframe txframe;
    
    // Notify elevator car and call boxes
    txframe.id = (MSCAN_CAR_ID | MSCAN_FL1_ID | MSCAN_FL2_ID | MSCAN_FL3_ID);
    txframe.priority = 0x01;
    txframe.length = 3;
    txframe.payload[0] = CMD_LOCATION;
    txframe.payload[1] = floor;
    txframe.payload[2] = DIRECTION_STATIONARY;  // Just set to stationary for now, this will change later
    CANsend(&txframe);
}




/*
 * Controller functionality
 * - Send elevator location messages to callboxes
 * - Listen for button press messages
 */
void controller() {
    byte sw1_pressed = 0, sw2_pressed = 0;
    byte rxmessage[PAYLOAD_SIZE];   // Received data payload 
    byte button_floor, button_direction;
    char *button_floor_str, *button_direction_str;
    
    byte update_lcd = 1;
    byte cycle_count = 0;
    word car_height, distance;
    byte cur_floor;
    char buf[64];
    byte b;
    
    dist_init();
    
    for(;;) {
        distance = dist_read();
        if ( distance > (7*5*CM_PER_FLOOR) ) {
            car_height = 999;
            cur_floor = 0;
            led7_write(led7_bars[1]);
        } else {
            car_height = (10*distance)/4;
            cur_floor = 1 + ((car_height / 10) / CM_PER_FLOOR);
            led7_write(led7_table[cur_floor]);
            update_floor(cur_floor);
        }
        
        cycle_count++;
        
        if ( cycle_count == 10 ) {
            update_lcd = 1;
            cycle_count = 0;
        }
        
        if ( update_lcd ) {
            update_lcd = 0;
            
            if ( cur_floor == 0 ) {
                lcd_goto(0x00);
                lcd_puts("No car   ");
            } else {
                itoa(car_height, 10, 3, "", buf);   
                lcd_goto(0x00);
                lcd_puts(buf);
                lcd_puts("mm/F"); 
                itoa(cur_floor, 10, 2, "", buf);   
                lcd_puts(buf+1);
            }
        }
        
        
        runSerialCAN();
        /*
        while ( sci_bytesAvailable() ) {
        	sci_readByte(&b);
			lcd_putc(b);  
        }		*/
        
        if(data_available()) {
            
            CANget(rxmessage);
            
            switch(rxmessage[0]) {
                case CMD_BUTTON_CALL:
                    button_floor = rxmessage[1];
                    button_direction = rxmessage[2];
                    
                    switch(button_floor) {
                    case FLOOR1:
                        button_floor_str = "1";
                        break;
                    case FLOOR2:
                        button_floor_str = "2";
                        break;
                    case FLOOR3:
                        button_floor_str = "3";
                        break;
                    default:
                        break;
                    }
                    switch(button_direction) {
                    case DIRECTION_UP:
                        button_direction_str = "up  ";
                        break;
                    case DIRECTION_DOWN:
                        button_direction_str = "down";
                        break;
                    case DIRECTION_STATIONARY:
                        button_direction_str = "stat";
                        break;
                    default:
                        break;
                    }
                    
                    lcd_goto(0x10); // Start at second line
                    lcd_puts("Floor");
                    lcd_puts(button_floor_str);
                    lcd_puts(" Dir ");
                    lcd_puts(button_direction_str);
                    break;
                case CMD_BUTTON_CAR:
                    
                    break;
                case CMD_DISP_APPEND:
                    
                    break;
                case CMD_ERROR:
                    
                    break;
                default:
                    lcd_goto(0x10); // Start at second line
                    lcd_puts("Unknown command");
                    break;
            }
        }
        msleep(100);
    }
}

/*
 * Callbox functionality
 * - Listen for button presses, and accept elevator location messages
 */
 
void button_up(byte my_floor) {	 
	CANframe txframe;	// Transmitted CAN frame
	LED1 = 1; 
	// Message to controller; up button pressed
	txframe.id = MSCAN_CTL_ID;
	txframe.priority = 0x01;
	txframe.length = 3;
	txframe.payload[0] = CMD_BUTTON_CALL;
	txframe.payload[1] = my_floor;
	txframe.payload[2] = BUTTON_UP;
	CANsend(&txframe);
}
     
void button_down(byte my_floor) {
	CANframe txframe;	// Transmitted CAN frame
	LED2 = 1;
	// Message to controller; down button pressed
	txframe.id = MSCAN_CTL_ID;
	txframe.priority = 0x01;
	txframe.length = 3;
	txframe.payload[0] = CMD_BUTTON_CALL;
	txframe.payload[1] = my_floor;
	txframe.payload[2] = BUTTON_DOWN;
	CANsend(&txframe);
}
 
 			  
static byte sw1_pressed = 0;
static byte sw2_pressed = 0;

void callbox(byte my_floor) {
    byte rxmessage[PAYLOAD_SIZE];   // Received data payload
    static byte floor, direction;
    
    floor = 0xFF;   // Start at false floor
    direction = DIRECTION_STATIONARY;   // Assume starting car direction is stationary
    
    if(SW1 && !sw1_pressed) {  
		sw1_pressed = 1;
    	button_up(my_floor);
    }
    if(SW2 && !sw2_pressed) { 
		sw2_pressed = 1;
        button_down(my_floor);
    }
    if(!SW1) sw1_pressed = 0;
    if(!SW2) sw2_pressed = 0;
    
    
        
    runSerialCAN();
        
        
    if(data_available()) {
        
        CANget(rxmessage);
        
        switch(rxmessage[0]) {
            case CMD_LOCATION:
                floor = rxmessage[1];
                direction = rxmessage[2];
                
                LCDclear();
                LCDprintf("Floor: %d\nDir: %d", floor, direction);
                break;
            case CMD_BUTTON_CALL:
            	rxmessage[1] == DIRECTION_UP ? button_up(my_floor) : button_down(my_floor);
            	break;
            case CMD_ERROR:
                LCDclear();
                LCDprintf("Error condition\nreceived!");
                break;
            default:
                LCDclear();
                LCDputs("Unknown command");
                break;
        }
        
        if(floor == my_floor) {
            LED1 = 0; LED2 = 0;
        }
    }   
    msleep(100);
}
