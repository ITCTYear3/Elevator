/*       SPI module macros and functions       */
/* setup to use with MAX5512/MAX5513 8-bit DAC */
#ifndef _SPI_H
#define _SPI_H

#include <mc9s12c32.h>

// SPI baud rates (8MHz bus clock)
#define BAUD_500KHZ         0x03
#define BAUD_1MHZ           0x02
#define BAUD_2MHZ           0x01
#define BAUD_4MHZ           0x00

// SPI interrupt macros
#define SPI_INT_ENABLE      (SPICR1_SPIE = 1)   // Enable SPI interrupts
#define SPI_INT_DISABLE     (SPICR1_SPIE = 0)   // Disable SPI interrupts
#define SPI_TX_INT_ENABLE   (SPICR1_SPTIE = 1)  // Enable SPI transmit interrupt
#define SPI_TX_INT_DISABLE  (SPICR1_SPTIE = 0)  // Disable SPI transmit interrupt

#define SPI_MODF_TRIG       ((SPISR >> 4) & 0x01)   // True if MODF fault has occurred


void SPIinit(void);
unsigned char SPIgetputc(char);
void SPIputc(char);
unsigned char SPIgetc(void);

#pragma MESSAGE DISABLE C4301   // "Inline expansion done for function call" warning message disable (compiler option)
/* Initialize SPI module */
void SPIinit(void) {
    SPIBR = BAUD_1MHZ;  // Set baud rate
    SPICR2_MODFEN = 1;  // Enable slave select with master mode
    SPICR1_CPHA = 1;    // Change clock phase
    
    // master mode, slave select is output in master mode, active high clock polarity, MSB first
    SPICR1 = ( SPICR1_MSTR_MASK | SPICR1_SSOE_MASK | SPICR1_CPOL_MASK & ~SPICR1_CPOL_MASK | SPICR1_LSBFE_MASK & ~SPICR1_LSBFE_MASK );
    SPICR1_SPE = 1;     // Enable SPI module
}

#pragma MESSAGE DISABLE C4002   // "Result not used" warning message disable for returning SPIDR value
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
    return SPIgetputc(0x00);
}

#endif // _SPI_H