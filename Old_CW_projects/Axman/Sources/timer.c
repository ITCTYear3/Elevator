   
#include <hidef.h>
#include "derivative.h"
#include <hidef.h>
#include "derivative.h"



#include "timer.h"



void timer_init() {
	
	
	// Enable timer module
	TSCR1_TEN = 1;
	
	// Ignore WAIT mode
	TSCR1_TSWAI = 0;
	
	// Ignore freeze mode
	TSCR1_TSFRZ = 0;
	
	// Disable fast-flag clear
	TSCR1_TFFCA = 0;
	
	
	
	// Ch7 input capture
	TIOS_IOS7 = 0;
	
	 /*
	// Ch7 capture rising and falling edges
	TCTL3_EDG7B = 1;	   
	TCTL3_EDG7A = 1;

	// Ch7 enable interrupt
    TIE_C7I = 1;
    
    */
    
    
    // Disble overflow interrupt
    TSCR2_TOI = 0;
    
    // Disable counter reset
    TSCR2_TCRE = 0;
    
    // No prescale
    TSCR2_PR = 0;
    
    
    
    
    // Enable pulse accumulator module
    PACTL_PAEN = 1;
    
    // Gated counter mode
    PACTL_PAMOD = 1;
    
    // High-level sensitive
    PACTL_PEDGE = 0;
    
    // PACLK clock source
    PACTL_CLK = 1;
    
    // Disable overflow interrupt
    PACTL_PAOVI = 0;
    
    // Disable accumulator interrupt
    PACTL_PAI = 0;
    
}



		   /*

interrupt VectorNumber_Vtimch7
void isr_Timer7() {   
	SET_BITS(TFLG1,(1 << 7));
}



interrupt VectorNumber_Vtimovf
void isr_timerOverflow() {
	TFLG2_TOF = 1;
	++ovfCount;
}


interrupt VectorNumber_Vtimpaie
void isr_timerOverflow() {
	TFLG2_TOF = 1;
	++ovfCount;
}
*/