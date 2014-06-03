#include <hidef.h>
#include "derivative.h"

#include "timer.h"
#include "dist.h"


void dist_init() {
    
    PACTL_PAEN = 1;     // Enable pulse accumulator module
    PACTL_PAMOD = 1;    // Gated counter mode
    PACTL_PEDGE = 0;    // High-level sensitive
    PACTL_CLK = 1;      // PACLK clock source
    PACTL_PAOVI = 0;    // Disable overflow interrupt
    PACTL_PAI = 0;      // Disable accumulator interrupt
    
    PTT_PTT6 = 0;
    DDRT_DDRT6 = 1;
}

word dist_read() {
    byte volatile i;    // Must be volatile or else compiler will remove the for loop below!
    
    PACNT = 0;  // Reset pulse accumulator count
    PAFLG_PAIF = 1;
    
    // Generate 10us trigger pulse
    PTT_PTT6 = 1;
    for (i=0; i<5; ++i);
    PTT_PTT6 = 0;
    
    while ( !PAFLG_PAIF );
    
    if ( PAFLG_PAOVF ) {
        return -1;
    }
    
    return PACNT;
}
