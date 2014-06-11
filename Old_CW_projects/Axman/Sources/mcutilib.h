#ifndef _MCUTILIB_H
#define _MCUTILIB_H

#include <hidef.h>
#include "derivative.h"

// This module contains portable, commonly-used MCU utilities


// ************************************************************************************************


typedef byte bitfield8;
typedef word bitfield16;


// General-purpose bit macros

#define LOW(value)  					((value) & 0xFF)
#define SET_BITS(var, mask)   			((var) |= (mask))
#define CLEAR_BITS(var, mask)   		((var) &= LOW(~(mask)))
#define FLIP_BITS(var, mask)   			((var) ^= (mask))
//#define FORCE_BITS(var, mask, value)	((var) = ((var) & LOW(~(mask))) | ((value) & (mask)))


//#define JOIN(a,b)		a ## b
#define JOIN(a,b)		_JOIN(a,b)
#define _JOIN(a,b)		a ## b

typedef struct {
	byte *data;
	byte head;
	byte tail;
	byte count;
	byte size;
} RingBuf;

byte ringEmpty(volatile RingBuf *ring);  
byte ringFull(volatile RingBuf *ring);  
void ringPut(volatile RingBuf *ring, byte value); 
byte ringTake(volatile RingBuf *ring);

		/*	
extern char numbuf[60];	   

#define ITOA10(n) itoa((n), 10, 5, "",   numbuf)
#define ITOA8(n)  itoa((n), 16, 2, "0x", numbuf) 
#define ITOA16(n) itoa((n), 16, 4, "0x", numbuf)  
#define ITOAFF(n) itoa((n), 16, 2, "",   numbuf)
		   */

void reverse(char *str, unsigned char start, unsigned char end);
char* itoa(int n, unsigned char radix, unsigned char width, char *prefix, char* buf);
int atoi(char* str, unsigned char start, unsigned char numChars);


void _strcpy(char *dest, char *src);

char _strcmp(char *s1, char *s2);

void _strcat(char *dest, char *src);

unsigned char _strlen(char *str);


 
void delay_ms(volatile unsigned int ms);

void delayMicros(volatile unsigned int us);


extern byte asin[];
extern byte nSins;

byte interpolate(byte value, byte* table, byte compression);

#endif