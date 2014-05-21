										 

#include "spi.h"



void spi_init() {		 
	
	// Disable interrupts
	SPICR1_SPIE = 0;
	SPICR1_SPTIE = 0;	
	
	// Enable master mode
	SPICR1_MSTR = 1;  	
	
	// Set clock polarity & phase (i.e. SPI mode 0 .. 3)
	SPICR1_CPOL = 0;  
	SPICR1_CPHA = 0;
	
	// Enable automatic SS operation
	SPICR1_SSOE = 1;		  
	
	// Transmit MSB first
	SPICR1_LSBFE = 0;
					  	
	// Enable mode fault detection (required for SS operation)
	SPICR2_MODFEN = 1; 	 
					  	
	// Disable bidirectional output buffer
	SPICR2_BIDIROE = 0;		 
					  	
	// SPI is not affected by WAIT
	SPICR2_SPISWAI = 0;
					   
	// 	Disable bidirectional I/O		   
	SPICR2_SPC0 = 0;    		   
		   
	// Baud rate 
	// See datasheet pg. 419 
	
	// BusClock = 8MHz
	// Divisor = (SPPR+1) * 2^(SPR+1)	
	// BaudRate = BusClock / Divisor
	// Set SPPR = 0	
	// BaudRate = 8MHz / 2^(SPR+1)
	// Set SPR = 0
	// BaudRate = 8 MHz / 2^1	  
	// BaudRate = 4 MHz
	
	SPIBR_SPPR = 0;
	SPIBR_SPR = 0; 	
	//SPIBR =  ((SPIBR_SPPR << SPIBR_SPPR_BITNUM) & SPIBR_SPPR_MASK) | (SPIBR_SPR & SPIBR_SPR_MASK);	  					  
					  
	// Enable SPI module
	SPICR1_SPE = 1;
		
}




void spi_write(byte b) {	   
	while ( ! SPISR_SPTEF );
	SPIDR = b;	 
}


