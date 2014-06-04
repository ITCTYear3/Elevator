/* Timer module functions */

#include <mc9s12c32.h>
#include "timer.h"


// Global timer overflow counter, updated by TCNT_Overflow_ISR interrupt handler
static word volatile timer_overflow_count;

/* Initialize timer module */
void timer_init(void) {
    
    TSCR1_TEN = 1;      // Enable timer module
    TSCR1_TSWAI = 0;    // Ignore WAIT mode
    TSCR1_TSFRZ = 1;    // Stop TCNT while in freeze mode (during debug)
    TSCR1_TFFCA = 1;    // Use fast-flag clear mode
    TSCR2_TOI = 0;      // Disable overflow interrupt
    TSCR2_TCRE = 0;     // Disable TCNT reset on successful TC7 output compare event
    SET_TCNT_PRESCALE(TCNT_PRESCALE_1); // Set timer prescaler to 1 (TCNT at 8MHz)
    
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
    if(!(TSCR1 & TSCR1_TEN_MASK)) TSCR1_TEN = 1;
    
    TC(TC_SLEEP) = TCNT + OC_DELTA_1MS; // Preset channel register
    TC_OC(TC_SLEEP);                    // Enable channel as output compare
    
    for(i=0; i < ms; i++) {
        while(!(TFLG1 & (1 << TC_SLEEP)));  // Wait for event
        TC(TC_SLEEP) += OC_DELTA_1MS;       // Rearm channel register, clearing TFLG as well
    }
}

/* Microsecond sleep timer */
void usleep(word us) {
    word i;
    
    // Enable timer module if not already enabled
    if(!(TSCR1 & TSCR1_TEN_MASK)) TSCR1_TEN = 1;
    
    TC(TC_SLEEP2) = TCNT + OC_DELTA_1US; // Preset channel register
    TC_OC(TC_SLEEP2);                    // Enable channel as output compare
    
    for(i=0; i < us; i++) {
        while(!(TFLG1 & (1 << TC_SLEEP2)));  // Wait for event
        TC(TC_SLEEP2) += OC_DELTA_1US;       // Rearm channel register, clearing TFLG as well
    }
}

/* TCNT overflow interrupt handler */
interrupt VectorNumber_Vtimovf
void TCNT_Overflow_ISR(void) {
#ifdef FAST_FLAG_CLR
    (void)TCNT;             // Clear timer overflow flag by reading TCNT (fast flag clear enabled)
#else
    TFLG2 = TFLG2_TOF_MASK; // Clear timer overflow flag by writing a one to it
#endif
    
    timer_overflow_count++;
}
