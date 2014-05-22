#include <hidef.h>
#include "derivative.h"

#include "mcutilib.h"

#include "lcd.h"

//#include "adc.h"

#include "dist.h"



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


#define CM_PER_FLOOR 15

word car_height;  
byte floor;

void main() {

	//byte i;
	word d;
	char buf[64];
	
	//adc_init();		 
	
	dist_init();
	
	lcd_init();
	
	
    welcome();	
				   
	lcd_clear();  	
	lcd_home(); 
	

	for(;;) {
	
		d = dist_read();
		if ( d > (7*4*CM_PER_FLOOR) ) {
			car_height = 999;
			floor = 0;
		} else {
			car_height = (10*d)/7;		
			floor = 1 + ((car_height / 10) / CM_PER_FLOOR);	   
		}	
	
		itoa(car_height, 10, 3, "", buf);   
		lcd_goto(0x00);
		lcd_puts(buf);
		lcd_puts("mm/F"); 
		itoa(floor, 10, 2, "", buf);   
		lcd_puts(buf+1);
		delay_ms(100);
	
	
	}
}