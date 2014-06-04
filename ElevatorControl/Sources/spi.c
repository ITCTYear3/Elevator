/* SPI module functions */
/* Only setup for sending, not receiving */

#include <mc9s12c32.h>
#include "spi.h"

void spi_init() {
    
    // Disable interrupts
    SPICR1_SPIE = 0;
    SPICR1_SPTIE = 0;
    
    SPICR1_MSTR = 1;    // Enable master mode
    
    // Set clock polarity & phase (i.e. SPI mode 0 .. 3)
    SPICR1_CPOL = 0;
    SPICR1_CPHA = 0;
    
    SPICR1_SSOE = 1;    // Enable automatic SS operation
    SPICR1_LSBFE = 0;   // Transmit MSB first
    SPICR2_MODFEN = 1;  // Enable mode fault detection (required for SS operation)
    SPICR2_BIDIROE = 0; // Disable bidirectional output buffer
    SPICR2_SPISWAI = 0; // SPI is not affected by WAIT
    SPICR2_SPC0 = 0;    // Disable bidirectional I/O
    
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
    
    SPICR1_SPE = 1; // Enable SPI module
}

void spi_write(byte data) {
    while(!SPISR_SPTEF);
    SPIDR = data;
}
