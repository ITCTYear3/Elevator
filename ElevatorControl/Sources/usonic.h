/*            LCD module macros             */
/* For Cytron HC-SOR4 Ultrasonic sensor module */
#ifndef _USONIC_H
#define _USONIC_H

#include "utils.h"
                
/*******************DEFINES********************************/
#define TC_TRIGGER  5
#define TC_ECHO     4

#define TRIGGER_LENGTH   10   // Length of time needed to trigger a measurement (10us)

/*******************MACROS*********************************/
#define TIME_IN(counts)     counts/148     // Returns distance in inches (counts must be in 1us resolution)
#define TIME_CM(counts)     counts/58      // Returns distance in centimeters(counts must be in 1us resolution)



/*******************PROTOTYPES*****************************/

void usonic_init(void);
word usonic_getDistance(void);

#endif // _USONIC_H