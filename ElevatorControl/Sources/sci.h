
// Serial Communications Interface


#ifndef _SCI_H
#define _SCI_H  

#include <mc9s12c32.h>


// Rx buffer size
#define SCI_RXBUF_SIZE 	64

// Tx buffer size
#define SCI_TXBUF_SIZE 64

// Baud rate
#define BUSCLK			8000000
#define SCI_CLK			BUSCLK
#define SCI_BAUDRATE	(unsigned long)9600	
#define INIT_SCIBD		SCI_CLK / ( 16 * SCI_BAUDRATE)


#define LCD_SERIAL


void sci_init(void);
word sci_bytesAvailable(void);
word sci_readByte(byte *b);
word sci_readBytes(byte *buf, word nBytes);
word sci_sendByte(byte b);
word sci_sendBytes(byte *buf, word nBytes);

#endif