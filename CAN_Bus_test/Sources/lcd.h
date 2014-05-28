/*            LCD module macros             */
/* For Hitachi HD44780 character LCD module */
#ifndef _LCD_H
#define _LCD_H

#include "utils.h"


#define LCD_MAX_BUFSIZ      32  // Max character buffer size for LCDprintf() (32 chars per line)

#define LCD_PORT            PORTA
#define LCD_PORT_DDR        DDRA
#define LCD_PORT_DDR_BITS   ( DDRA_BIT0_MASK | DDRA_BIT1_MASK | DDRA_BIT2_MASK | DDRA_BIT3_MASK | \
                              DDRA_BIT4_MASK | DDRA_BIT5_MASK | DDRA_BIT6_MASK )

#define LCD_E_BIT           PORTA_BIT4_MASK     // LCD enable (clock)
#define LCD_RS_BIT          PORTA_BIT5_MASK     // LCD register select (instruction or data)
#define LCD_RW_BIT          PORTA_BIT6_MASK     // LCD read/write
#define LCD_BUS_BITS        ( PORTA_BIT0_MASK | PORTA_BIT1_MASK | PORTA_BIT2_MASK | PORTA_BIT3_MASK )

/*****************************************************************************/

// Low-level LCD macros
#define LCD_E_LO            CLR_BITS(LCD_PORT,LCD_E_BIT)
#define LCD_E_HI            SET_BITS(LCD_PORT,LCD_E_BIT)

#define LCD_RS_IR           CLR_BITS(LCD_PORT,LCD_RS_BIT)   // LCD instruction register
#define LCD_RS_DR           SET_BITS(LCD_PORT,LCD_RS_BIT)   // LCD data register

#define LCD_RW_WRITE        CLR_BITS(LCD_PORT,LCD_RW_BIT)
#define LCD_RW_READ         SET_BITS(LCD_PORT,LCD_RW_BIT)

#define LCD_BUS(value)      FORCE_BITS(LCD_PORT,LCD_BUS_BITS,(value))   // Write data on LCD bus
#define LCD_BUS_DDR(dir)    FORCE_BITS(LCD_PORT_DDR,LCD_BUS_BITS,(dir)) // Set DDR for LCD bus bits

#define LO_NIBBLE(value)    ((value) & 0x0F)        // Get lower nibble of byte
#define HI_NIBBLE(value)    (((value) & 0xF0) >> 4) // Get upper nibble of byte


// Commands to LCD module
#define LCD_CMD_CLEAR           0x01
#define LCD_CMD_HOME            0x02
#define LCD_CMD_ENTRY           0x04
#define LCD_CMD_DISPLAY         0x08
#define LCD_CMD_CD_SHIFT        0x10
#define LCD_CMD_FUNCTION        0x20
#define LCD_CMD_CGRAMADDR       0x40
#define LCD_CMD_SET_DDADDR      0x80

// Settings for LCD_CMD_ENTRY
#define LCD_ENTRY_MOVE_DISPLAY  0x01
#define LCD_ENTRY_MOVE_CURSOR   0x00
#define LCD_ENTRY_INC           0x02
#define LCD_ENTRY_DEC           0x00

// Settings for LCD_CMD_DISPLAY
#define LCD_DISPLAY_BLINK       0x01
#define LCD_DISPLAY_NOBLINK     0x00
#define LCD_DISPLAY_CURSOR      0x02    
#define LCD_DISPLAY_NOCURSOR    0x00
#define LCD_DISPLAY_ON          0x04
#define LCD_DISPLAY_OFF         0x00

// Settings for LCD_CMD_CD_SHIFT (shift cursor or display without changing data)
#define LCD_CD_SHIFT_RIGHT      0x04
#define LCD_CD_SHIFT_LEFT       0x00
#define LCD_CD_SHIFT_DISPLAY    0x08
#define LCD_CD_SHIFT_CURSOR     0x00

// Settings for LCD_CMD_FUNCTION
#define LCD_FUNCTION_5X10FONT   0x04
#define LCD_FUNCTION_5X8FONT    0x00
#define LCD_FUNCTION_2LINES     0x08
#define LCD_FUNCTION_1LINE      0x00
#define LCD_FUNCTION_8BIT       0x10
#define LCD_FUNCTION_4BIT       0x00


#define LCD_BF_MASK             0x80    // Busy flag bit mask
#define LCD_AC_MASK             0x7F    // Address counter bit mask

// GC RAM specific masks
#define LCD_CHAR_NUM_MASK       0x38    // Character code number mask (middle 3 bits)
#define LCD_LINE_MASK           0x07    // Character pattern line mask (lower 3 bits)
#define LCD_LINE_DATA_MASK      0x1F    // Character pattern line data mask (lower 5 bits)

#define LCD_DDADDR_LINE2        0x40    // DD RAM address for beginning of second line

/*****************************************************************************/

void LCDinit(void);
void LCDclear(void);
void LCDhome(void);
void LCDbksp(void);
void LCDputc(char);
void LCDputs(const char *);
void LCDprintf(const char *, ... );
byte LCDgetc(void);
byte LCDaddress(void);
byte LCDbusy(void);



#endif // _LCD_H