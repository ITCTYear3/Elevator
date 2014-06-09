/* HC-SR04 Ultrasonic Sensor
 * User Manual: https://docs.google.com/document/d/1Y-yZnNhMYy7rwhAgyL_pfa39RsB-x2qR4vP8saG73rE
 * 3mm resolution
 */

#include <mc9s12c32.h>
#include "dist.h"
#include "timer.h"  // usleep()

#define TRIG_PIN    PTT_PTT6    // Using PT6 gpio pin for trigger

// Pulse accumulator overflow counter, updated by PACNT_Overflow_ISR interrupt handler
static word volatile pulse_overflow_count;


/* Initialize pulse accumulator module */
void dist_init(void) {
    
    PACTL_PAMOD = 1;    // Gated counter mode
    PACTL_PEDGE = 0;    // High-level sensitive
    PACTL_CLK = 0;      // Keep using prescaled Bus clk as the clock source
    PACTL_PAOVI = 0;    // Disable overflow interrupt
    PACTL_PAI = 0;      // Disable accumulator interrupt
    TIOS_IOS7 = 0;      // Ch7 input capture (echo pin)
    PACTL_PAEN = 1;     // Enable pulse accumulator module
    
    pulse_overflow_count = 0;
    
    // Setup trigger pin gpio
    DDRT_DDRT6 = 1;
    TRIG_PIN = 0;
}

/* Count echo pin pulse length */
// TODO: return value converted to mm distance length
word dist_read(void) {
    word volatile count;
    dword volatile count2;
    byte i;
    
    PACNT = 0;  // Reset pulse accumulator count
#ifndef FAST_FLAG_CLR
    PAFLG = PAFLG_PAIF_MASK;    // Clear interrupt edge flag by writing a one to it
#endif
    
    // Generate 10us trigger pulse
    TRIG_PIN = 1;
    delay_us(10); // ** Microsecond delay function is not accurate, unsure as to why
    //for(i=0; i<15; i++);    // Equivalent to 10us delay
    TRIG_PIN = 0;
    
    while(!PAFLG_PAIF); // Wait for falling edge on echo pin
    
    /* Calculate distance in mm:
     * PACLK is bus clock divided by 64 (8MHz / 64 = 125kHz)
     * Each PACLK time tick is 8us
     * time ticks * 8 = pulse width(us)
     * pulse width(us) / 58 = distance(cm)
     * distance(cm) * 10 = distance(mm)
     */
    //count = 10*((PACNT*8) / 58);
    /* Using binary scaling to get a good estimate
     * (PACNT*(80/58)*8)/8 = PACNT*(80/58)
     */
    count = PACNT;
    count2 = 11 * (dword)count;
    count = (word)(count2 / 8);
    
    // Check in case of PACNT overflow
    if(PAFLG_PAOVF) {
        PAFLG = PAFLG_PAOVF_MASK;   // Clear overflow flag by writing a one to it
        return 0;
    }
    
    return count;
}

/* Current PACNT overflow count */
word get_pulse_overflow_count(void) {
    return pulse_overflow_count;
}

/* PACNT overflow interrupt handler */
interrupt VectorNumber_Vtimpaovf
void PACNT_Overflow_ISR(void) {
#ifdef FAST_FLAG_CLR
    (void)PACNT;    // Clear timer overflow flag by reading PACNT (fast flag clear enabled)
#else
    PAFLG = PAFLG_PAOVF_MASK;   // Clear overflow flag by writing a one to it
#endif
    
    pulse_overflow_count++;
}
