                 
#include <hidef.h>
#include "derivative.h"

/* Timer module functions */

#include <mc9s12c32.h>
#include "utils.h"
#include "timer.h"


// Global timer overflow counter, updated by TCNT_Overflow_ISR interrupt handler
static word volatile timer_overflow_count;

/* Initialize timer module */
void timer_init(void) {
    
       
    // Setup pulse accumulator on ch7  
      
    // Enable timer module
  	TSCR1_TEN = 1;    	
  	// Ignore WAIT mode
  	TSCR1_TSWAI = 0;   	
  	// Ignore freeze mode
  	TSCR1_TSFRZ = 0;        	
  	// Disable fast-flag clear
  	TSCR1_TFFCA = 1;
  	
  	// Ch7 input capture
  	TIOS_IOS7 = 0;

    // Disble overflow interrupt
    TSCR2_TOI = 0;        
    // Disable counter reset
    TSCR2_TCRE = 0;        
    // No prescale
    //TSCR2_PR = 0;   
      
             
    
    
    // Setup millisecond timer
                       
    // TOI currently disabled due to issues with the interrupt firing too often
    //TOI_ENABLE;                 // Enable TCNT overflow interrupt
    
    SET_TCNT_PRESCALE(TCNT_PRESCALE_1);     // Set timer prescaler to 8 (TCNT at 1MHz)
    //SET_BITS(TSCR1,TSCR1_INIT);             // Set timer operation modes and enable timer
    
    timer_overflow_count = 0;   // Reset overflow counter
    
      
    
    
    
}

/* Current timer overflow count */
word get_overflow_count(void) {
    return timer_overflow_count;
}

/*      Milisecond sleep timer       */
/* Uses output compare timer channel */
/* to count in milisecond increments */
void msleep(word ms) {
    word i;
    
    // Enable timer module if not already enabled
    if(!(TSCR1 & TSCR1_TEN_MASK)) EnableTimer;
    
    TC(TC_SLEEP) = TCNT + OC_DELTA_1MS; // Preset channel register
    TC_OC(TC_SLEEP);                    // Enable channel as output compare
    
    for(i=0; i < ms; i++) {
        while(!(TFLG1 & (1 << TC_SLEEP)));  // Wait for event
        TC(TC_SLEEP) += OC_DELTA_1MS;       // Rearm channel register, clearing TFLG as well
    }
}


/*****************************************************************************/



/* TCNT overflow interrupt handler */
interrupt VectorNumber_Vtimovf void TCNT_Overflow_ISR(void) {
#ifdef FAST_FLAG_CLR
    (void)TCNT;             // Clear timer overflow flag by reading TCNT (fast flag clear enabled)
#else
    TFLG2 = TFLG2_TOF_MASK; // Clear timer overflow flag by writing a one to it
#endif
    
    timer_overflow_count++;
}

