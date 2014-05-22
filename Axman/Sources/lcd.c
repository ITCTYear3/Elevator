#include <hidef.h>
#include "derivative.h"

#include "mcutilib.h"

#include "spi.h"
#include "lcd.h"


byte lcd_reg = 0;


byte lcd_data_h;
byte lcd_data_l;



byte lcd_mode;


			
byte lcd_rs;




void lcd_goto(byte rowcol);



void lcd_putc(byte b) {

	byte lcd_state = LCD_STATE_READY; 
	lcd_data_l = b & 0x0F;
	lcd_data_h = (b >> 4) & 0x0F;
	
	// Intercept special characters
	if ( lcd_rs == LCD_RS_DATA ) {
		switch ( b ) {
			case '\n':
				lcd_goto(0x10);
				return;
		}
	}

	while ( lcd_state != LCD_STATE_IDLE ) {
		switch ( lcd_state ) {			 
			case LCD_STATE_READY:  
				SET_BITS(lcd_reg, LCD_EN_MASK);
				lcd_state = LCD_STATE_SETUP_H;
				break;		 
			case LCD_STATE_SETUP_H:
				FORCE_BITS(lcd_reg, LCD_RS_MASK, ( lcd_rs == LCD_RS_DATA ? ONES_BYTE : ZEROS_BYTE ) ); 
				FORCE_BITS(lcd_reg, LCD_DATA_MASK, lcd_data_h );
				lcd_state = LCD_STATE_VALID_H;
				break;	   
			case LCD_STATE_VALID_H:	   
				CLEAR_BITS(lcd_reg, LCD_EN_MASK);
				lcd_state = LCD_STATE_HOLD_H;
				break;	   
			case LCD_STATE_HOLD_H:
		//delay_ms(5);	   
				SET_BITS(lcd_reg, LCD_EN_MASK);
				lcd_state = ( lcd_mode == LCD_NIBBLE_MODE ? LCD_STATE_SETUP_L : LCD_STATE_IDLE );
				break;  	 
			case LCD_STATE_SETUP_L:	  
				FORCE_BITS(lcd_reg, LCD_RS_MASK, ( lcd_rs == LCD_RS_DATA ? ONES_BYTE : ZEROS_BYTE ) ); 
				FORCE_BITS(lcd_reg, LCD_DATA_MASK, lcd_data_l );
				lcd_state = LCD_STATE_VALID_L;
				break;	   
			case LCD_STATE_VALID_L:	  	   
				CLEAR_BITS(lcd_reg, LCD_EN_MASK);
				lcd_state = LCD_STATE_HOLD_L;
				break;	   
			case LCD_STATE_HOLD_L:	
	//	delay_ms(5); 
				SET_BITS(lcd_reg, LCD_EN_MASK);
				lcd_state = LCD_STATE_IDLE;
				break;
		}
		spi_write(lcd_reg);
	//	delay_ms(15);
	}
}



		    

void lcd_goto(byte rowcol) {	  								
	byte cmd = rowcol & 0x0F;
	if ( rowcol & 0x10 ) {
		cmd += 0x40;
	}
	cmd |= 0x80;   
	lcd_rs = LCD_RS_INST;
	lcd_putc(cmd);			
	lcd_rs = LCD_RS_DATA;
}




void lcd_clear() {	  				
	lcd_rs = LCD_RS_INST;
	lcd_putc(0x01);	
    delay_ms(15);		
	lcd_rs = LCD_RS_DATA;
}


void lcd_home() {	  				
	lcd_rs = LCD_RS_INST;
	lcd_putc(0x02);		
    delay_ms(15);	
	lcd_rs = LCD_RS_DATA;
}




void lcd_puts(char *s) {
	char *c;
	for ( c = s; *c != '\0'; ++c ) {
		lcd_putc(*c);
	}
}


void lcd_init() {

	
	spi_init();
	
	
						  
	lcd_rs = LCD_RS_INST;
	
	lcd_mode = LCD_BYTE_MODE;

	// Set 4bit mode
    delay_ms(15);
	lcd_putc(0x30); 
	lcd_putc(0x30);    
	lcd_putc(0x30);   
	lcd_putc(0x20); 
	lcd_mode = LCD_NIBBLE_MODE;
			
	// 2 lines, 5x8 font
	lcd_putc(0x28);	
		
	// Display off, cursor off, blink off	
	lcd_putc(0x08);
		
	// Clear screen, cursor home
	lcd_putc(0x01); 
    delay_ms(15);	
		
	// Shift cursor right, don't shift screen
	lcd_putc(0x06); 	
				 
	// Display on
	lcd_putc(0x0C); 		
			   	
	// Switch to data register
	lcd_rs = LCD_RS_DATA;
}
