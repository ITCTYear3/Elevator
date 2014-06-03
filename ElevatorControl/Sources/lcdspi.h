
#include <hidef.h>
#include "derivative.h"

								
#define ONES_BYTE 	(byte)0xFF
#define ZEROS_BYTE	(byte)0x00
								  
#define ONES_WORD 	(word)0xFFFF
#define ZEROS_WORD	(word)0x0000


#define LCD_EN_MASK 	0b10000000
#define LCD_RS_MASK 	0b01000000  
#define LCD_DATA_MASK 	0b00001111



#define LCD_BYTE_MODE 	0
#define LCD_NIBBLE_MODE 1

				 

#define LCD_RS_INST 	0
#define LCD_RS_DATA		1




#define LCD_STATE_IDLE 		0
#define LCD_STATE_READY		1	  
#define LCD_STATE_SETUP_H	2    
#define LCD_STATE_VALID_H	3   
#define LCD_STATE_HOLD_H	4    
#define LCD_STATE_SETUP_L	5 
#define LCD_STATE_VALID_L	6    
#define LCD_STATE_HOLD_L	7


void lcd_goto(byte rowcol);



void lcd_putc(byte b);


void lcd_clear(void);

void lcd_home(void);

void lcd_goto(byte rowcol);

void lcd_puts(char *s);
void lcd_init(void);
