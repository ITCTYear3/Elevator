/* Generic utility and bit twiddling macros */
#ifndef _UTILS_H
#define _UTILS_H

typedef unsigned char   byte;
typedef unsigned int    word;
typedef unsigned long   dword;


#define LOW(value)      ((value) & 0xFF)            // Get lower byte of word
#define HIGH(value)     (((value) >> 4) & 0xFF)     // Get upper byte of word


#define SET_BITS(port,mask)     ((port) |= (mask))          // Set port bits specified in mask
#define CLR_BITS(port,mask)     ((port) &= LOW(~(mask)))    // Clear port bits specified in mask
#define FLIP_BITS(port,mask)    ((port) ^= (mask))          // Toggle port bits specified in mask

// Force port bits specified in mask to value
#define FORCE_BITS(port,mask,value) ((port) = ((port) & LOW(~(mask)) | ((value) & (mask))))

// Force port bits specified in mask to value (word)
#define FORCE_WORD(port,mask,value) ((port) = ((port) & ((~(mask)) & 0xFFFF) | ((value) & (mask))))


// Preprocessor concatenation helper macros
#define CAT2(x,y)   x ## y
#define CAT(x,y)    CAT2(x,y)


#endif // _UTILS_H