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
#include "serialcan.h"
#include "sci.h"
#include "dist.h"
#include "pid.h"
#include "mctrl.h"
#include "simpleQueue.h"
#include "lcd.h"
#include "lcdspi.h"
#include "led7.h"
#include "mcutilib.h"

#define LED1    PTS_PTS2
#define LED2    PTS_PTS3
#define SW1     !PTJ_PTJ6
#define SW2     !PTJ_PTJ7

#define CM_PER_FLOOR 15


// All of these defines are auto chosen based on which node define is selected
//#define USE_LCD     // HD44780 32 character LCD display board
//#define USE_LCD2    // HD44780 32 character LCD display board connected via SPI (Chris' Axman)
//#define USE_LED7    // 7-segment custom callbox display board attachment


// Set local node ID (unique to each node)
//#define MSCAN_NODE_ID   MSCAN_CTL_ID
//#define MSCAN_NODE_ID   MSCAN_CAR_ID
//#define MSCAN_NODE_ID   MSCAN_FL1_ID
#define MSCAN_NODE_ID   MSCAN_FL2_ID
//#define MSCAN_NODE_ID   MSCAN_FL3_ID


#if MSCAN_NODE_ID == MSCAN_CTL_ID

    void controller(void);
    #define RUN()   controller();
    
    #ifndef USE_LCD
    #define USE_LCD
    #endif

#elif MSCAN_NODE_ID == MSCAN_CAR_ID    

    void car(void);
    #define RUN()   car();
    
    #ifndef USE_LCD
    #define USE_LCD
    #endif

#elif MSCAN_NODE_ID & (MSCAN_FL1_ID | MSCAN_FL2_ID | MSCAN_FL3_ID)

    void callbox(byte my_floor);
    #if MSCAN_NODE_ID == MSCAN_FL1_ID
        #define RUN()   callbox(FLOOR1);
        
        #ifndef USE_LCD2
        #define USE_LCD2
        #endif
        
    #elif MSCAN_NODE_ID == MSCAN_FL2_ID
        #define RUN()   callbox(FLOOR2);
        
        #ifndef USE_LED7
        #define USE_LED7
        #endif
        
    #elif MSCAN_NODE_ID == MSCAN_FL3_ID
        #define RUN()   callbox(FLOOR3);
        
        #ifndef USE_LCD
        #define USE_LCD
        #endif
        
    #endif

#else

    #error "One of the node IDs must be selected!"

#endif


#pragma MESSAGE DISABLE C1420   // "Result of function-call is ignored" warning message disable for sci_sendBytes()
void main(void) {
    
    //byte b;
    
    DDRS_DDRS2 = 1; DDRS_DDRS3 = 1; // Enable LEDs
    DDRJ_DDRJ6 = 0; DDRJ_DDRJ7 = 0; // Enable switches
    
    timer_init();
    CANinit(MSCAN_NODE_ID);
    
    // Clear all MSCAN receiver flags (by setting bits)
    CANRFLG = (CANRFLG_RXF_MASK | CANRFLG_OVRIF_MASK | CANRFLG_CSCIF_MASK | CANRFLG_WUPIF_MASK);
    
    sci_init();
    
#ifdef USE_LCD
    msleep(16); // wait 16ms before init LCD
    LCDinit();  // initialize LCD, cursor should be visible with blink after
#endif
    
#ifdef USE_LCD2
    lcd_init();
#endif
    
#ifdef USE_LED7
    led7_init();
    led7_write(LED7_HBARS);
#endif
    
    EnableInterrupts;
    
    sci_sendBytes((byte*)"Ready", 6);   // Send ready message indicator via serial
    
    for(;;) {
        /*
        while ( sci_bytesAvailable() ) {
            sci_readByte(&b);
            lcd_putc(b);  
        }*/
        RUN();
    }
}

/*
 * Transmits the floor that the current elevator car is on
 * to the internal panel and to each call box
 */
#pragma MESSAGE DISABLE C1420   // "Result of function-call is ignored" warning message disable for CANsend()
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
    byte button_pressed;
	byte next_floor;
    char *button_floor_str, *button_direction_str;
    
    byte update_lcd = 1;
    byte cycle_count = 0;
    word car_height, distance;  // car height in cm, distance measurement in mm
    byte cur_floor, last_floor = 0;
    //byte b;   // used for debug manual frame sending testing
    
    dist_init();
    //mctrl_init();   // Initialize servo motor controller
    
    for(;;) {
        
        distance = dist_read()/2;   // div2 is a temporary kludge
        last_floor = cur_floor;
        if ( distance > (7*5*CM_PER_FLOOR) ) {
            car_height = 999;
            cur_floor = 0;
#ifdef USE_LED7
            led7_write(led7_bars[1]);
#endif
        } else {
            car_height = (10*distance)/4;
            cur_floor = 1 + ((car_height / 10) / CM_PER_FLOOR);
#ifdef USE_LED7
            led7_write(led7_table[cur_floor]);
#endif
            if ( cur_floor != last_floor ) {
                update_floor(cur_floor);
            }
            
            // Update PID controller feedback value
            //pid_feedback((car_height/10) * 100);    // scale value up to something we can work with on the bench
        }
        
        //mctrl_update();
        
        cycle_count++;
        
        if ( cycle_count == 10 ) {
            update_lcd = 1;
            cycle_count = 0;
        }
        
        if ( update_lcd ) {
            update_lcd = 0;
            
#ifdef USE_LCD
            if ( cur_floor == 0 ) { 
                LCDclear();
                LCDputs("No car");
            } else {
                LCDclear();
                LCDprintf("%dmm/F%d", car_height, cur_floor);
            }
#endif
        }
        
        // CAN bus <-> serial link
        // Check for new incoming messages and send out received messages via serial
        runSerialCAN(MSCAN_NODE_ID);
        /*
        while ( sci_bytesAvailable() ) {
            sci_readByte(&b);
            lcd_putc(b);  
        } */
        
        if(data_available()) {
            
            CANget(rxmessage);
            
            switch(rxmessage[0]) {
            case CMD_BUTTON_CALL:
                button_pressed = rxmessage[1];
                
                addToQueue(button_pressed);
                next_floor = peekNextFloor();
                
                switch(cur_floor) {
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
                
                if(next_floor == cur_floor){
                    button_direction_str = "stat";
                }else if(next_floor > cur_floor){
                    button_direction_str = "up  ";
                }else {
                    button_direction_str = "down";
                }
                
#ifdef USE_LCD
                LCDhome();
                LCDprintf("\nFloor%s Dir %s", button_floor_str, button_direction_str);
#else
#ifdef USE_LCD2
                lcd_goto(0x10); // Start at second line
                lcd_puts("Floor");
                lcd_puts(button_floor_str);
                lcd_puts(" Dir ");
                lcd_puts(button_direction_str);
#endif
#endif
                break;
            case CMD_BUTTON_CAR:
				button_pressed = rxmessage[1];
				if(button_pressed == BUTTON_STOP){
					// call emergency stop function
				}else if(button_pressed < BUTTON_DOOR_CLOSE)
					addToQueue(button_pressed);
					
				next_floor = peekNextFloor();
                
                switch(cur_floor) {
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
                
                if(next_floor == cur_floor){
                    button_direction_str = "stat";
                }else if(next_floor > cur_floor){
                    button_direction_str = "up  ";
                }else {
                    button_direction_str = "down";
                }
                
#ifdef USE_LCD
                LCDhome();
                LCDprintf("\nFloor%s Dir %s", button_floor_str, button_direction_str);
#else
#ifdef USE_LCD2
                lcd_goto(0x10); // Start at second line
                lcd_puts("Floor");
                lcd_puts(button_floor_str);
                lcd_puts(" Dir ");
                lcd_puts(button_direction_str);
#endif
#endif
				break;
            case CMD_DISTANCE:
                distance = (rxmessage[1] << 8) | rxmessage[2];
                pid_feedback(distance);
                break;
            case CMD_ERROR:
                
                break;
            default:
#ifdef USE_LCD
                LCDclear();
                LCDputs("\nUnknown command");
#else
#ifdef USE_LCD2
                lcd_goto(0x10); // Start at second line
                lcd_puts("Unknown command");
#endif
#endif
                break;
            }
        }
        
        delay_ms(100);
    }
}


/*
 * Elevator car functionality
 * - 
 */
void car(void) {
    byte ret;
    byte sw1_pressed = 0, sw2_pressed = 0;
    char *command, *floor, *direction;
    byte cur_floor;
    int elevatorDirection = 0xFF;
    
    CANframe txframe;               // Transmitted CAN frames
    byte rxmessage[PAYLOAD_SIZE];   // Received data payload
    
    cur_floor = 0;
    
    // Message to controller; floor 1
    txframe.id = MSCAN_CTL_ID;
    txframe.priority = 0x01;
    txframe.length = 2;
    txframe.payload[0] = CMD_BUTTON_CAR;
    txframe.payload[2] = DIRECTION_STATIONARY; 
    
    
    if(PORTB == FLOOR1) {           
         if (BUTTON_FL1 > cur_floor) 
         {
             elevatorDirection = DIRECTION_DOWN;
         } 
         else if (BUTTON_FL1 < cur_floor) 
         {
             elevatorDirection = DIRECTION_UP;
         } 
         else 
         {
             elevatorDirection = DIRECTION_STATIONARY;
         }
         
  	     txframe.payload[1] = BUTTON_FL1;
  	     txframe.payload[2] = elevatorDirection;
                
         ret = CANsend(&txframe);
         if(ret) {
             // Message could not be sent!
         }
    }
    if(PORTB == FLOOR2) {          
         if (BUTTON_FL2 > cur_floor) 
         {
             elevatorDirection = DIRECTION_DOWN;
         } 
         else if (BUTTON_FL2 < cur_floor) 
         {
             elevatorDirection = DIRECTION_UP;
         } 
         else 
         {
             elevatorDirection = DIRECTION_STATIONARY;
         }
         
  	     txframe.payload[1] = BUTTON_FL2;
  	     txframe.payload[2] = elevatorDirection;
                
         ret = CANsend(&txframe);
         if(ret) {
             // Message could not be sent!
         }
    } 
    // +1 is used because the physical button is bit 3, logical number 4.
    if(PORTB == (FLOOR3 + 1)) {             
         if (BUTTON_FL3 > cur_floor) 
         {
             elevatorDirection = DIRECTION_DOWN;
         } 
         else if (BUTTON_FL3 < cur_floor) 
         {
             elevatorDirection = DIRECTION_UP;
         } 
         else 
         {
             elevatorDirection = DIRECTION_STATIONARY;
         }
         
  	     txframe.payload[1] = BUTTON_FL3;
  	     txframe.payload[2] = elevatorDirection;
                
         ret = CANsend(&txframe);
         if(ret) {
             // Message could not be sent!
         }
    }
     
    if(PORTB == DROPEN) {
  	     txframe.payload[1] = BUTTON_DOOR_OPEN;
  	     txframe.payload[2] = elevatorDirection;
                
         ret = CANsend(&txframe);
         if(ret) {
             // Message could not be sent!
         }
    }
     
    if(PORTB == DRCLOSE) {   
         
  	     txframe.payload[1] = BUTTON_DOOR_CLOSE;
  	     txframe.payload[2] = elevatorDirection;
                
         ret = CANsend(&txframe);
         if(ret) {
             // Message could not be sent!
         }
    }
    if(PORTB == ESTOP) {     
         
  	     txframe.payload[1] = BUTTON_STOP;
  	     txframe.payload[2] = elevatorDirection;
                
         ret = CANsend(&txframe);
         if(ret) {
             // Message could not be sent!
         }
    }
    // CAN bus <-> serial link
    // Check for new incoming messages and send out received messages via serial
    runSerialCAN(MSCAN_NODE_ID);
    
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
                goto car_cmd_error;
        }
        
        switch(rxmessage[1]) {
            case FLOOR1:
                floor = "1";
                cur_floor = 1;
                break;
            case FLOOR2:
                floor = "2";  
                cur_floor = 2;
                break;
            case FLOOR3:
                floor = "3"; 
                cur_floor = 3;
                break;
            default:
                // Command didn't match known commands!
                goto car_cmd_error;
        }
        
        
        // Turn off LED when car arrives at requested floor
        if ( rxmessage[0] == CMD_LOCATION ) {           
            if( rxmessage[1] == FLOOR1 ) {
                LED1 = 0;
            }
            if( rxmessage[1] == FLOOR2 ) {
                LED2 = 0;
            }
            /*if( rxmessage[1] == FLOOR3 ) {
                LED3 = 0;
            }*/
        }
        
#ifdef USE_LED7
        led7_write(led7_table[cur_floor]); 
#endif
#ifdef USE_LCD
        LCDhome();
        LCDclear();
        LCDprintf("Command: %s\nFloor%s Dir: %s", command, floor, direction);
#endif
        
        return;
        
car_cmd_error:
        //LCDhome();
        //LCDclear();
        //LCDprintf("Error in\ncommand");
        return;
        
    }
}


/*
 * Callbox functionality
 * - Listen for button presses, and accept elevator location messages
 */

void button_up(byte my_floor) {
    CANframe txframe;
    LED1 = 1;
#ifdef USE_LCD
    LCDclear();
    LCDprintf("Floor: %d\nDir: %s", my_floor, "up");
#endif
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
    CANframe txframe;
    LED2 = 1;
#ifdef USE_LCD
    LCDclear();
    LCDprintf("Floor: %d\nDir: %s", my_floor, "down");
#endif
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
    word distance;
    CANframe txframe;   // Transmitted CAN frame
    
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
    
    runSerialCAN(MSCAN_NODE_ID);
    
    if(data_available()) {
        
        CANget(rxmessage);
        
        switch(rxmessage[0]) {
            case CMD_LOCATION:
                floor = rxmessage[1];
                direction = rxmessage[2];
                
#ifdef USE_LCD
                LCDclear();
                LCDprintf("Floor: %d\nDir: %d", floor, direction);
#endif
#ifdef USE_LED7
                led7_write(led7_table[floor]);
#endif
                break;
            case CMD_BUTTON_CALL:
                rxmessage[1] == DIRECTION_UP ? button_up(my_floor) : button_down(my_floor);
                break;
            case CMD_ERROR:
#ifdef USE_LCD
                LCDclear();
                LCDprintf("Error condition\nreceived!");
#endif
                break;
            default:
#ifdef USE_LCD
                LCDclear();
                LCDputs("Unknown command");
#endif
                break;
        }
        
        // Turn off indicator LED once car has reached local floor
        if(floor == my_floor) {
            LED1 = 0; LED2 = 0;
        }
        
    }
    
    // Sonar sensor currently attached to callbox 1
    // Send off distance message to controller node
    if (my_floor == FLOOR1) {
        distance = dist_read()/2; 
        
        txframe.id = MSCAN_CTL_ID;
        txframe.priority = 0x01;
        txframe.length = 3;
        txframe.payload[0] = CMD_DISTANCE;
        txframe.payload[1] = (distance & 0xFF00) >> 8;         
        txframe.payload[2] = (distance & 0x00FF);
        CANsend(&txframe);
    }
    
    msleep(100);
}
