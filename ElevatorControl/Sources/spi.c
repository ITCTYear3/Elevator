/*
 * spi.c
 * SPI module macros and functions
 * Compatible with with MAX5512/MAX5513 8-bit DAC
 */

#include <mc9s12c32.h>
#include "spi.h"

//#pragma MESSAGE DISABLE C4301   // "Inline expansion done for function call" warning message disable (compiler option)


/* Initialize SPI module */
void SPIinit(void) {
    
    SPICR1_MSTR = 1;    // Enable master mode
    
    // Set clock polarity & phase (i.e. SPI mode 0 .. 3)
    SPICR1_CPOL = 0;    // Active-high SCK
    SPICR1_CPHA = 0;    // Sampling on odd edges of SCK
    
    SPICR1_LSBFE = 0;   // Transmit MSB first
    SPICR1_SSOE = 1;    // Enable automatic SS operation
    SPICR2_MODFEN = 1;  // Enable slave select with master mode
    SPICR2_BIDIROE = 0; // Disable bidirectional output buffer
    SPICR2_SPISWAI = 0; // SPI is not affected by WAIT
    SPICR2_SPC0 = 0;    // Disable bidirectional I/O
    
    SPIBR = BAUD_1MHZ;  // Set baud rate
    
    SPICR1_SPE = 1;     // Enable SPI module
}

//#pragma MESSAGE DISABLE C4002   // "Result not used" warning message disable for returning SPIDR value
/* Read and write char on SPI */
unsigned char SPIgetputc(char c) {
    while(!SPISR_SPTEF); // Wait for data register to become empty
    SPIDR = c;
    
    /* Byte shifted out and new byte shifted in */
    
    while(!SPISR_SPIF); // Wait until byte shifted in
    return SPIDR;
}

#pragma MESSAGE DISABLE C1420   // "Result of function-call is ignored" warning message disable for SPIgetputc()
/* Write char to SPI */
void SPIputc(char c) {
    SPIgetputc(c);
}

/* Read char from SPI */
unsigned char SPIgetc(void) {
    return SPIgetputc(0x00);    // Write a dummy value while reading incoming data byte
}
