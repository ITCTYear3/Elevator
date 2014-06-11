#include <hidef.h>
#include "derivative.h"

#include "timer.h"

#include "dist.h"




void dist_init() {
	
	timer_init();
	
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