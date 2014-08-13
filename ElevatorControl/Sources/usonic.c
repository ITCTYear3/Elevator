/* Ultra Sonic Sensors module functions 
   Author: Mark Mahony
   Date: May 15, 2014
*/ 

#include <mc9s12c32.h>
#include "utils.h"
#include "timer.h"
#include "usonic.h"

word first_count;
word second_count;

void usonic_init(void) {
    TC_OC(TC_TRIGGER); 
    TC_IC(TC_ECHO);
}

// Returns distance in mm. +- 3mm (according to manufacturer spec)
word usonic_getDistance(void) {

    word measured_count;
      
    first_count = 0;
    second_count = 0;
    
    // Enable interrupts on the echo channel.
    TC_INT_ENABLE(TC_ECHO);      
    // Set input compare to detect rising edge, beginning of distance measurement pulse.
    SET_IC_EDGE(TC_ECHO,IC_DETECT_RISING);      
        
    // Enable timer module if not already enabled
    //if(!(TSCR1 & TSCR1_TEN_MASK)) EnableTimer;
     
    // Start TTL-high pulse.
    SET_OC_ACTION(TC_TRIGGER, OC_HI);
    FORCE_OC_ACTION(TC_TRIGGER);
    
    // Set 10us timer.
    TC(TC_TRIGGER) = TCNT + (OC_DELTA_1US * TRIGGER_LENGTH); 
        
    while(!(TFLG1 & (1 << TC_TRIGGER)));  // Wait for pulse to finish.
    
    // Pulse finished, pull OC output low.
    SET_OC_ACTION(TC_TRIGGER, OC_LO);
    FORCE_OC_ACTION(TC_TRIGGER);
    
    // Wait for the pulse to finish, then calculate distance.
    while ((first_count == 0) || (second_count == 0)) ;
    
    // Multiplication of 10 is used to increase the resolution and return the value in mm, instead of cm.
    measured_count = (second_count - first_count) * 10;
    
    return TIME_CM(measured_count);
    
    
}

/* Ultrasonic Echo interrupt handler */
interrupt VECTOR_NUM(TC_VECTOR(TC_ECHO)) void sonic_ISR(void) {

    if (first_count == 0) {
        // Detected rising edge of measurement pulse.
        first_count = TC(TC_ECHO);
        //reset_overflow_count();
    
        // Look for the falling edge.          
        SET_IC_EDGE(TC_ECHO,IC_DETECT_FALLING); 
    } 
    else if (second_count == 0) { 
        second_count = TC(TC_ECHO); // + (get_overflow_count() * (dword)65535);
        TC_INT_DISABLE(TC_ECHO); 
    }
}