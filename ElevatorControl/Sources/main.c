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
#include "lcdspi.h"
#include "protocol.h"   // CAN node IDs and payload data IDs     

#include "dist.h"   
#include "led7.h"
#include "mcutilib.h"

#define LED1    PTS_PTS2
#define LED2    PTS_PTS3
#define SW1     !PTJ_PTJ6
#define SW2     !PTJ_PTJ7


// Define the type of node here
#define   MSCAN_NODE_ID     MSCAN_CTL_ID             
//#define   MSCAN_NODE_ID     MSCAN_CAR_ID
//#define   MSCAN_NODE_ID     MSCAN_FL1_ID   
//#define   MSCAN_NODE_ID     MSCAN_FL2_ID
//#define   MSCAN_NODE_ID     MSCAN_FL3_ID



#if MSCAN_NODE_ID & MSCAN_CTL_ID
                                 
    void controller(void);
    #define RUN() controller();

#elif MSCAN_NODE_ID & MSCAN_CAR_ID    
                                    
    void car(void);       
    #define RUN() car();

#elif MSCAN_NODE_ID & (MSCAN_FL1_ID | MSCAN_FL2_ID | MSCAN_FL2_ID)

    void callbox(void);   
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
    
    
    
    dist_init();
    
    led7_init();
    led7_write(LED7_HBARS);
    
    lcd_init();
    
    delay_ms(500);
    
    
    EnableInterrupts;
    
    for(;;) {
    
      RUN();
      
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







#define N_FLOORS 3

// Transmits the floor that the current elevator car is on
// to the internal panel and to each call box

void update_floor(byte f) {

  byte i;            
  CANframe txframe;

    // Notify car
  txframe.id = MSCAN_CAR_ID;
  txframe.priority = 0x01;
  txframe.length = 3;
  txframe.payload[0] = CMD_LOCATION;
  txframe.payload[1] = f;
  txframe.payload[2] = DIRECTION_STATIONARY;        
    CANsend(&txframe);
        

    // Notify call boxes
    for ( i = 0; i < N_FLOORS; ++i ) {
    txframe.id = MSCAN_FL1_ID << i;
    txframe.priority = 0x01;
    txframe.length = 3;
    txframe.payload[0] = CMD_LOCATION;
    txframe.payload[1] = f;
    txframe.payload[2] = DIRECTION_STATIONARY;
    CANsend(&txframe);                  
    }
}






#define CM_PER_FLOOR 15

word car_height;
byte cur_floor;

void controller() {

    byte ret;
  byte sw1_pressed = 0, sw2_pressed = 0;
  char *command, *floor, *direction;

  //CANframe txframe1, txframe2;    // Transmitted CAN frames
    byte rxmessage[PAYLOAD_SIZE];   // Received data payload
    
    
    byte update_lcd;
    byte cycle_count;

    //byte i;
    word d;
    char buf[64];
    
    //adc_init();        

    //leds_init();
    
    
    //CANinit(1);     
    //CANinit(MSCAN_NODE_ID);
    
     
                   
    lcd_clear();
    lcd_home(); 
                       

    cycle_count = 0;                   
                       
    update_lcd = 1;


    for(;;) {
    
        d = dist_read();
        if ( d > (7*5*CM_PER_FLOOR) ) {
            car_height = 999;
            cur_floor = 0;
            led7_write(led7_bars[1]);
        } else {
            car_height = (10*d)/4;      
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
      
      //LCDhome();
      //LCDclear();
      //LCDprintf("Command: %s\nFloor%s Dir: %s", command, floor, direction);

      lcd_home();
      lcd_clear();
            lcd_puts("Cmd: ");
            lcd_puts(command);
            lcd_puts(" Floor: ");
            itoa(*floor, 10, 2, "", buf);
            lcd_puts(buf);
            lcd_goto(0x10);
            lcd_puts("Dir: ");
            lcd_puts(direction);


            return;

    cmd_error:
      //LCDhome();
      //LCDclear();
      //LCDprintf("Error in\ncommand");


      lcd_home();
      lcd_clear();
            lcd_puts("Error in command");  

      }


        delay_ms(10);



    }



}


