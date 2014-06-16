
// Serial Communications Interface


#include <mc9s12c32.h>


#include "sci.h"

#include "mcutilib.h"


// Incoming serial data buffer	   
byte rxBufData[SCI_RXBUF_SIZE];
volatile RingBuf rxBuf = { 
/*	.data =*/ rxBufData,
/*	.head =*/ 0,  
/*	.tail =*/ 0,
/*	.count =*/ 0,	  
/*	.size =*/ SCI_RXBUF_SIZE
};
	
// Outgoing serial data buffer
byte txBufData[SCI_TXBUF_SIZE];
volatile RingBuf txBuf = { 
/*	.data =*/ txBufData,
/*	.head =*/ 0,  
/*	.tail =*/ 0,
/*	.count =*/ 0,	  
/*	.size =*/ SCI_TXBUF_SIZE
};


void sci_init() {    		  
    SCIBD = INIT_SCIBD;		// Set baud rate (see sci.h)
    SCICR1 = 0x00;  		// Plain 8N1 UART configuration
    SCICR2 = (SCICR2_RIE_MASK | SCICR2_RE_MASK | SCICR2_TE_MASK);  // Enable Tx, Rx, and Rx interrupt
}


// Returns the number of bytes available in the Rx queue
word sci_bytesAvailable() {
	return ringAvailable(&rxBuf);
}


// Reads a byte from the queue into *b, if possible
// Returns the number of bytes actually read (i.e. 0 or 1)
word sci_readByte(byte *b) {	   
	if ( ringEmpty(&rxBuf) ) {
		return 0;
	}
	DisableInterrupts;
	*b = ringTake(&rxBuf);
	EnableInterrupts;
    return 1; 
}
	
	
// Reads bytes from the Rx queue into a buffer
// Returns the number of bytes that were actually read
word sci_readBytes(byte *buf, word nBytes) {
	word i;
	for ( i = 0; i < nBytes; ++i ) {
		if ( (sci_readByte(buf+i) ) == 0 ) {
			break;
		}
	}
	return i;
}	



// Queues a byte for transmission  
// Returns the number of bytes that were actually written
word sci_sendByte(byte txByte) {   
	if ( ringFull(&txBuf) ) {
		return 0;
	} 				    
	DisableInterrupts;
	ringPut(&txBuf, txByte);
	EnableInterrupts;
	SCICR2_SCTIE = 1;	 // Enable Tx interrupt
	return 1;
}


// Queues a buffer of bytes for transmission 
// Returns the number of bytes that were actually written 
// Aborts immediately after a byte fails to write
word sci_sendBytes(byte *buf, word nBytes) {
	word i;
	for ( i = 0; i < nBytes; ++i ) { 
		if ( sci_sendByte(buf[i]) == 0 ) {
			break;
		}
	}
	return i;
}


// SCI interrupt
// Transmits bytes from the Tx queue and adds recieved bytes to the Rx queue
// There are three conditions that will cause this interrupt to fire:
//   1) A new byte has been recieved and is in the Rx data register waiting to be read
//   2) The Tx register is empty and accepting a new byte
//   3) A transmission has been completed
// Conditions 2 and 3 are only enabled when there are bytes to transmit.
// Once all available bytes have been transmitted, those conditions are disabled 
// to prevent the interrupt from firing continuously. 
interrupt VectorNumber_Vsci
void isr_SCI() {  	
	// Rx register full, new byte available	   
	if ( SCISR1_RDRF ) {   		
		// Rx buffer not full, add new byte 
		if ( ! ringFull(&rxBuf) ) {
			ringPut(&rxBuf, SCIDRL);
		}
	}
	// Tx register empty, ready to send new byte	
	if ( SCISR1_TDRE ) {
		// Tx buffer not empty, send one byte   	 
		if ( ! ringEmpty(&txBuf) ) {	 
			SCIDRL = ringTake(&txBuf);
			SCICR2_TCIE = 1;  			
		}
		// Tx buffer empty, disable TDRE interrupts 
		else {
			SCICR2_SCTIE = 0;
		}
	} 	
	// Tx register empty, disable Tx interrupts		  
	if ( SCISR1_TC ) {	  
		SCICR2_TCIE = 0; 
	}
}
