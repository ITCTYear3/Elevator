/* Timer module functions */

#include <mc9s12c32.h>
#include "timer.h"

// Timer overflow counter, updated by TCNT_Overflow_ISR interrupt handler
static word volatile timer_overflow_count;


/* Initialize timer module */
void timer_init(void) {
    
    TSCR1_TSWAI = 0;    // Continue incrementing TCNT while in WAIT mode
    TSCR1_TSFRZ = 0;    // Continue incrementing TCNT while in freeze mode
#ifdef FAST_FLAG_CLR
    TSCR1_TFFCA = 1;    // Use fast-flag clear mode
#else
    TSCR1_TFFCA = 0;
#endif
    TSCR2_TOI = 0;      // Disable overflow interrupt
    TSCR2_TCRE = 0;     // Disable TCNT reset on successful TC7 output compare event
    SET_TCNT_PRESCALE(TCNT_PRESCALE_1); // Set timer prescaler to 1 (TCNT at 8MHz)
    TSCR1_TEN = 1;      // Enable timer module
    
    timer_overflow_count = 0;
}

/* Current timer overflow count */
word get_overflow_count(void) {
    return timer_overflow_count;
}

/* Microsecond sleep timer */
void delay_us(word us) {
    
    TC_OC(TC_DELAY);    // Set timer channel to output compare
#if TIMETICKS_1US == 1
    TC(TC_DELAY) = TCNT + us;
#else
    TC(TC_DELAY) = TCNT + (TIMETICKS_1US * us); // Preset channel register
#endif
    
    while(!( TFLG1 & (1 << TC_DELAY) ));
}

/* Millisecond sleep timer */
void msleep(word ms) {
    word i;
    
    TC_OC(TC_DELAY);
#if TIMETICKS_1MS == 1
    TC(TC_DELAY) = TCNT + ms;
#else
    TC(TC_DELAY) = TCNT + TIMETICKS_1MS;    // Preset channel register
#endif
    
    for(i=0; i < ms; i++) {
        while(!(TFLG1 & (1 << TC_DELAY)));  // Wait for event
        TC(TC_DELAY) += TIMETICKS_1MS;      // Rearm channel register, clearing TFLG as well
    }
}
/*
void msleep(word ms) {
    
    TC_OC(TC_DELAY);    // Set timer channel to output compare
#if TIMETICKS_1MS == 1
    TC(TC_DELAY) = TCNT + ms;
#else
    TC(TC_DELAY) = TCNT + (TIMETICKS_1MS * ms); // Preset channel register
#endif
    
    while(!( TFLG1 & (1 << TC_DELAY) ));
}*/

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
