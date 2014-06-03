#include <hidef.h>
#include "derivative.h"


#include "mcutilib.h"



// -----------------------------------------------
//		Ring Buffer


#pragma INLINE
byte ringEmpty(volatile RingBuf *ring) {
	return ring->count == 0;
}

#pragma INLINE
byte ringFull(volatile RingBuf *ring) {
	return ring->count == ring->size;
}

#pragma MESSAGE DISABLE C2705 // Possible loss of data warning
#pragma INLINE
void ringPut(volatile RingBuf *ring, byte value) { 
	ring->data[(ring->head)++] = value;
	ring->head %= ring->size;
	++(ring->count);		  
}

#pragma INLINE
byte ringTake(volatile RingBuf *ring) {	 
	byte value;  
	value = ring->data[(ring->tail)++];
	ring->tail %= ring->size;
	--(ring->count);	 
	return value;	
}


// -----------------------------------------------
//		String utilities


// Used by itoa and variants to build numeric strings
char numbuf[60];


// Reverses the order of characters in a string between start and end indexes                     
void reverse(char *str, unsigned char start, unsigned char end) {
  char temp;    
  while ( start <= end ) {
    temp = str[start];
    str[start++] = str[end];
    str[end--] = temp;
  } 
}
 
 
// Integer to ascii                       
char* itoa(int n, unsigned char radix, unsigned char width, char *prefix, char* buf) {
  int q;
  unsigned char i = 0;
  unsigned char w = 0;
  unsigned char start;
  unsigned char end;
  
  // Append prefix string
  for ( i; prefix[i]; i++ ) {
    buf[i] = prefix[i];
  }      
  
  // Note number begining position
  start = i;
  
  	     // Convert number to ascii in reverse  
  for ( i; w < width; i++, w++ ) {
    q = (n % radix);
    buf[i] = '0' + q + (q > 9 ? 7 : 0);  // Handles the shift from '9'->'A'
    n /= radix;
  } 
  
  // Note number end position  
  end = i - 1;
    
  // Correct reversed conversion
  reverse(buf, start, end);
  
  buf[i] = '\0';
  return buf;
  
}



// Ascii to integer
int atoi(char* str, unsigned char start, unsigned char numChars) {
  unsigned char i;
  int n = 0;
  char c;
  
  for ( i = start; i < start + numChars; i++ ) {  
    c = str[i] - '0';
    if ( c > 0x0F ) {
      c -= 7;
    } 
    n <<= 4;
    n += c;     
  }          
  return n;  
}


// The string routines included here use 8-bit datatypes instead of ints a la string.h
// Performance may or may not be improved; should test this

void _strcpy(char *dest, char *src) {
 	while ( *src != '\0' ) {
 		*dest = *src;
 		++src;
 		++dest;
 	}
 	*dest = '\0';
}

char _strcmp(char *s1, char *s2) {
	while ( *s1 == *s2 && *s1 != '\0' && *s2 != '\0' ) {
		++s1;
		++s2;
	}
	return *s1 - *s2;
}

void _strcat(char *dest, char *src) {
	while ( *dest != '\0' ) {
		++dest;
	}
	while ( *src != '\0' ) {
		*dest = *src;
		++dest;
		++src;
	}
	*dest = '\0';
}

unsigned char _strlen(char *str) {
	char *c = str;
 	while ( *(c++) != '\0' );
 	return c - str;
}


// Only accurate for BUSCLK = 8MHz.. fix this

void delayMicros(volatile unsigned int t) {
  
  t *= 3;
  
  // Miniumum possible delay: 2.625 us
  if ( t < 3 ) {
    return;
  }
  
  // Special case for t = 3
  if ( t == 3 ) { 
    _asm("nop");
    return;
  }
  
  // At this point, 3 microseconds have elapsed 
  // So we should wait for t-3 more microseconds
  
  // There is an additional 1.0us overhead below
  // So we will subtract an extra microsecond
  t -= 4;
  
  #asm
  
  	pshx
  
    // 0.75us overhead
    ldx t              ; 6
   
    // 1us per loop
    loop:
    nop                 ; 1
    nop                 ; 1 
    nop                 ; 1
    dex                 ; 1
    beq done            ; 1
    bra loop            ; 3
    
    // 0.25us overhead
    done:               ; 2 from beq
    
    pulx
    
  #endasm
}


void delay_ms(volatile unsigned int ms) {
	while ( ms-- > 0 ) {
		delayMicros(997);
	}
}


byte asin[] = {
	2, 14, 25, 36, 50, 56, 62, 68, 73, 78, 84, 89, 94, 99, 104, 109, 115, 121, 127, 134, 141, 150, 163, 180 
};

// Table lookup with linear interpolation
// Useful for approximating functions without floating-point hardware
byte interpolate(byte value, byte* table, byte compression) {
	byte index = value / compression;
	byte result = table[index] + ((word)(table[index+1] - table[index]) * (value - index * compression)) / compression;
	return result;
}

