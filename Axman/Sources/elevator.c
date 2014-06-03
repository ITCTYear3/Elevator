#include <hidef.h>
#include "derivative.h"

#include "mcutilib.h"

#include "leds.h"

#include "lcdspi.h"

//#include "adc.h"

#include "dist.h"

#include "led7.h"

#include "mscan.h"

#include "protocol.h"

char *welcome_line0 = "    Elevator    ";
char *welcome_line1 = "                ";	 

byte welcome_delay[9] = { 200, 60, 40, 30, 30, 30, 40, 60, 200 };

void welcome() {   
	char i, j; 	  
	lcd_clear(); 	
	lcd_home();   			
	for ( i = 8; i >= 0; --i ) { 		  
		lcd_goto(0x00);
		for ( j = 0; j < i; 		++j ) 	lcd_putc(0xFF);
		for ( j = 0; j < 16-(2*i); 	++j ) 	lcd_putc(welcome_line0[i+j]);	
		for ( j = 0; j < i; 		++j )	lcd_putc(0xFF);
		lcd_goto(0x10);
		for ( j = 0; j < i; 		++j )	lcd_putc(0xFF);
		for ( j = 0; j < 16-(2*i); 	++j )	lcd_putc(welcome_line1[i+j]);	
		for ( j = 0; j < i; 		++j ) 	lcd_putc(0xFF);	  		
		delay_ms(welcome_delay[i]);		  			
	}  	
} 

void unwelcome() {
	byte i, j; 			    			
	for ( i = 0; i < 9; ++i ) { 			  
		lcd_goto(0x00);
		for ( j = 0; j < i; 		++j ) 	lcd_putc(0xFF);
		for ( j = 0; j < 16-(2*i); 	++j ) 	lcd_putc(welcome_line0[i+j]);	
		for ( j = 0; j < i; 		++j )	lcd_putc(0xFF);
		lcd_goto(0x10);
		for ( j = 0; j < i; 		++j )	lcd_putc(0xFF);
		for ( j = 0; j < 16-(2*i); 	++j )	lcd_putc(welcome_line1[i+j]);	
		for ( j = 0; j < i; 		++j ) 	lcd_putc(0xFF);	  		
		delay_ms(welcome_delay[i]);		  			
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

void main() {

 	byte ret;
  byte sw1_pressed = 0, sw2_pressed = 0;
  char *command, *floor, *direction;
  
  //CANframe txframe1, txframe2;    // Transmitted CAN frames
  dataMessage rxmessage;          // Received data payload
    
    
	byte update_lcd;
	byte cycle_count;

	//byte i;
	word d;
	char buf[64];
	
	//adc_init();		 
	
	leds_init();
	
	dist_init();
	
	led7_init();
	led7_write(LED7_HBARS);
	
	lcd_init();
	
	
	//CANinit(1);     
	CANinit(MSCAN_FL1_ID);
	
	leds_set(0, 0, 8, 8);	  
	leds_set(1, 2, 8, 8);
	
	
    welcome();	
				   
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
    
      CANget(&rxmessage);
      data_used();
      
     switch(rxmessage.payload[0]) {
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
      
      switch(rxmessage.payload[1]) {
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
      
      switch(rxmessage.payload[2]) {
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
	
		leds_run();
		
		
		
		
		
		
		
	
	}
	
	
	
}

