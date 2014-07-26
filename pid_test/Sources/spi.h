/*
 * spi.h
 * SPI module macros and functions
 * Compatible with with MAX5512/MAX5513 8-bit DAC
 */
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
unsigned char SPIgetputc(char c);
void SPIputc(char c);
unsigned char SPIgetc(void);

#endif // _SPI_H
