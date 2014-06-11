/*           LCD module functions           */
/* For Hitachi HD44780 character LCD module */

#include <mc9s12c32.h>
#include <stdio.h>      // for vsprintf()
#include <stdarg.h>     // for va_*() functions
#include "utils.h"
#include "timer.h"
#include "lcd.h"


static void LCDcmd(byte cmd);
static void LCDdata(byte data);
static void LCDcgen(byte cnum, byte *line_data);

/* Initialize LCD */
/* Wait at least 15ms after power-on */
void LCDinit(void) {
    FORCE_BITS(LCD_PORT_DDR,LCD_PORT_DDR_BITS,LCD_PORT_DDR_BITS);   // set LCD_PORT as outputs
    
    LCD_E_HI;
    LCD_BUS( 0x03 );    // wake-up display & sync
    LCD_E_LO;
    
    msleep(5);  // > 4.1ms
    
    LCD_E_HI;
    LCD_BUS( 0x03 );    // wake-up display & sync
    LCD_E_LO;
    
    msleep(1);  // > 100us
    
    LCD_E_HI;
    LCD_BUS( 0x03 );    // wake-up display & sync
    LCD_E_LO;
    
    LCD_E_HI;
    LCD_BUS( 0x02 );    // change to 4-bit mode
    LCD_E_LO;
    
    msleep(2);  // extra wait before configure (nessesary!)
    
    // LCD is now synced, configure the display
    LCDcmd( LCD_CMD_FUNCTION | LCD_FUNCTION_4BIT | LCD_FUNCTION_2LINES | LCD_FUNCTION_5X8FONT );
    LCDcmd( LCD_CMD_DISPLAY | LCD_DISPLAY_OFF );
    LCDclear();
    LCDcmd( LCD_CMD_ENTRY | LCD_ENTRY_MOVE_CURSOR | LCD_ENTRY_INC );    // increment address by one and shift cursor right at time of write
    LCDcmd( LCD_CMD_DISPLAY | LCD_DISPLAY_ON | LCD_DISPLAY_NOCURSOR | LCD_DISPLAY_NOBLINK );    // no cursor, no blink
    //LCDcmd( LCD_CMD_DISPLAY | LCD_DISPLAY_ON | LCD_DISPLAY_CURSOR | LCD_DISPLAY_BLINK );        // visible cursor with blink
}

/* Clear LCD screen */
void LCDclear(void) {
    LCDcmd(LCD_CMD_CLEAR);
    msleep(16);
}

/* Return to home position */
void LCDhome(void) {
    LCDcmd(LCD_CMD_HOME);
    msleep(16);
}

/* Delete last character */
void LCDbksp(void) {
    LCDputc(0x08);
}

#pragma MESSAGE DISABLE C1855   // Recursive function call warning disable (recursive LCDputc used for backspace sequence)
/* Write char to LCD */
void LCDputc(char c) {
    // Catch escape sequences
    switch(c) {
    case 0x07: /* bell (\a) */
        LCDclear();
        break;
    case 0x08: /* backspace (\b) */
        LCDcmd( LCD_CMD_SET_DDADDR | LCDaddress()-1 );
        LCDputc(' ');
        LCDcmd( LCD_CMD_SET_DDADDR | LCDaddress()-1 );
        break;
    case 0x09: /* tab (\t) */
        LCDputs("  ");  // double space tab width
        break;
    case 0x0A: /* newline (\n) */
        LCDcmd( LCD_CMD_SET_DDADDR | LCD_DDADDR_LINE2 );
        break;
    case 0x0D: /* carrage return (\r) */
        LCDhome();
        break;
    default:
        LCDdata(c);
        break;
    }
}

/* Write string to LCD */
void LCDputs(const char *str) {
    while(*str)
        LCDputc(*str++);
}

#pragma MESSAGE DISABLE C1420   // Function call result ignored warning disable (for vsprintf)
/* Write formatted string to LCD */
void LCDprintf(const char *fmt, ... ) {
    char buffer[LCD_MAX_BUFSIZ];
    
    va_list args;
    va_start(args, fmt);
    vsprintf(buffer, fmt, args);
    va_end(args);
    
    LCDputs(buffer);
}

/* Read LCD char (data register) */
byte LCDgetc(void) {
    byte c = 0;
    
    LCD_BUS_DDR(0); // set LCD_PORT data bits as inputs
    
    // preamble
    LCD_E_LO;
    LCD_RS_DR;      // data register for reading from CG RAM or DD RAM
    LCD_RW_READ;
    
    // read upper nibble
    LCD_E_HI;
    c |= (LCD_PORT & LCD_BUS_BITS) << 4;
    LCD_E_LO;
    
    // read lower nibble
    LCD_E_HI;
    c |= (LCD_PORT & LCD_BUS_BITS) & 0x0F;
    LCD_E_LO;
    
    LCD_BUS_DDR(LCD_BUS_BITS); // set LCD_PORT data bits back to output
    
    return c;
}

/* Read LCD address counter (instruction register) */
byte LCDaddress(void) {
    byte addr = 0;
    
    LCD_BUS_DDR(0);    // set LCD_PORT data bits as inputs
    
    // preamble
    LCD_E_LO;
    LCD_RS_IR;      // instruction register for reading busy flag & address
    LCD_RW_READ;
    
    // read upper nibble
    LCD_E_HI;
    addr |= (LCD_PORT & LCD_BUS_BITS) << 4;
    LCD_E_LO;
    
    // read lower nibble
    LCD_E_HI;
    addr |= (LCD_PORT & LCD_BUS_BITS) & 0x0F;
    LCD_E_LO;
    
    LCD_BUS_DDR(LCD_BUS_BITS); // set LCD_PORT data bits back to output
    
    return addr;
}

/* Read LCD busy flag */
byte LCDbusy(void) {
    return (LCDaddress() & LCD_BF_MASK) >> 7;  // return busy flag bit
}

/*****************************************************************************/

/* Send LCD command */
static void LCDcmd(byte cmd) {
    // preamble
    LCD_E_LO;
    LCD_RS_IR;
    LCD_RW_WRITE;
    
    // write upper nibble
    LCD_E_HI;
    LCD_BUS(HI_NIBBLE(cmd));
    LCD_E_LO;
    
    // write lower nibble
    LCD_E_HI;
    LCD_BUS(LO_NIBBLE(cmd));
    LCD_E_LO;
    
    // wait for command to finish
    msleep(1);
}

/* Send LCD data */
static void LCDdata(byte data) {
    // preamble
    LCD_E_LO;
    LCD_RS_DR;
    LCD_RW_WRITE;
    
    // write upper nibble
    LCD_E_HI;
    LCD_BUS(HI_NIBBLE(data));
    LCD_E_LO;
    
    // write lower nibble
    LCD_E_HI;
    LCD_BUS(LO_NIBBLE(data));
    LCD_E_LO;
    
    // wait for command to finish
    msleep(1);
}

/* Write to character generator RAM */
// cnum         3-bit value of character code number
// line_data    pointer to array of 5-bit values used for character pattern data (size 7)
static void LCDcgen(byte cnum, byte *line_data) {
    /***  For 5x7 character patterns  ***/
    /*** 8 possible custom characters ***/
    /*
     * CG RAM address bits 0-2 designate the line position within a character pattern
     * CG RAM address bits 3-5 designate the custom character code number (3 bits: 8 characters)
     * DD RAM data bits 0-2 correspond to CG RAM address bit 3-5
     * A '1' in CG RAM data means the pixel is lit, and a '0' mean unlit
     * The 8th character pattern line is the cursor position and
     *    display is determined by the logical OR of the 8th line and the cursor
     */
    
    byte line;     // 3-bit value of character pattern line position
    byte address;  // current position in DD RAM to be restored before returning
    
    // remember current position in DD RAM
    address = LCDaddress();
    
    // character code number must be in the range of 1-8
    if(cnum < 1)
        cnum = 1;
    else if(cnum > 8)
        cnum = 8;
    
    // position CG RAM address pointer in character table to the top line in character pattern
    // address pointer will auto increment if LCD entry is configured to increment during init
    LCDcmd( LCD_CMD_CGRAMADDR | ((cnum-1 << 3) & LCD_CHAR_NUM_MASK) | (0x00 & LCD_LINE_MASK) );
    
    // write all lines to CG RAM data (5 bits per line)
    for(line=0; line < 7; line++)
        LCDdata( line_data[line] & LCD_LINE_DATA_MASK );
    
    // restore address pointer to previous DD RAM location
    LCDcmd( LCD_CMD_SET_DDADDR | (address & LCD_AC_MASK) );
}
