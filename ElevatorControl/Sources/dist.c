/* HC SR04 Ultrasonic Sensor */

#include <mc9s12c32.h>
#include "timer.h"  // usleep()
#include "dist.h"

#define TRIG_PIN    PTT_PTT6    // Using PT6 gpio pin for trigger

void dist_init() {
    
    PACTL_PAEN = 1;     // Enable pulse accumulator module
    PACTL_PAMOD = 1;    // Gated counter mode
    PACTL_PEDGE = 0;    // High-level sensitive
    PACTL_CLK = 1;      // PACLK clock source
    PACTL_PAOVI = 0;    // Disable overflow interrupt
    PACTL_PAI = 0;      // Disable accumulator interrupt
    
    TIOS_IOS7 = 0;  // Ch7 input capture (echo pin)
    
    // Setup trigger pin gpio
    DDRT_DDRT6 = 1;
    TRIG_PIN = 0;
}

/* Count echo pin pulse length */
word dist_read() {
    byte volatile i;    // Must be volatile or else compiler will optimize out the for loop below!
    
    PACNT = 0;      // Reset pulse accumulator count
    PAFLG_PAIF = 1; // Clear interrupt edge flag by writing a one to it
    
    // Generate 10us trigger pulse
    TRIG_PIN = 1;
    usleep(10);
    TRIG_PIN = 0;
    
    while(!PAFLG_PAIF); // Wait for falling edge
    
    // Check in case of PACNT overflow
    if(PAFLG_PAOVF) {
        PAFLG_PAOVF = 1;    // Clear overflow flag by writing a one to it
        return -1;
    }
    
    return PACNT;
}
