#include <hidef.h>
#include "derivative.h"

#include "timer.h"

#include "dist.h"




void dist_init() {
	
	//timer_init();
	
	
      
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
	
	PTT_PTT6 = 0;
	DDRT_DDRT6 = 1;
	
	
}





word dist_read() {

	volatile word i;
	word r;
						
	PACNT = 0;
			
			
	PAFLG_PAIF = 1;	
	
	PTT_PTT6 = 1;		
	for ( i = 0; i < 5; ++i );   	
	PTT_PTT6 = 0;
	
	while ( ! PAFLG_PAIF );
	
	if ( PAFLG_PAOVF ) {
		return -1;
	}
	
	return PACNT;  
	

}