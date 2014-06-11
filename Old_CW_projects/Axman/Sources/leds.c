
#include <hidef.h>
#include "derivative.h"

#include "mcutilib.h"

#include "leds.h"





void leds_init() {
	
	DDRS_DDRS2 = 1;		
	DDRS_DDRS3 = 1;
			   
	PTS_PTS2 = 0;
	PTS_PTS3 = 0;


}



typedef struct LedMode_struct { 
	byte offset;
	byte onTime;
	byte offTime;
	byte count;
} LedMode;



LedMode ledMode[2];

						   				  
void leds_set(byte ch, byte offset, byte onTime, byte offTime) {
    ledMode[ch].offset = offset;								 
    ledMode[ch].onTime = onTime;
    ledMode[ch].offTime = offTime;
    ledMode[ch].count = 0; 
}

void leds_run() {
	byte i;	  
	byte count; 
	byte period;
	byte onTime;
	byte offTime;   
	for ( i = 0; i < 2; ++i ) {     
		period = ledMode[i].onTime + ledMode[i].offTime;
		count = ledMode[i].count - ledMode[i].offset;  
		if ( count >= 128 ) {
			count += period;								  
		}
		onTime = ledMode[i].onTime;
		offTime = ledMode[i].onTime + ledMode[i].offTime;
		//onTime %= period;								  
		if ( offTime > period ) {
			offTime -= period;
		}
	//	offTime %= period;	 
		if ( count < onTime ) { 
			CLEAR_BITS(PTS, (1<<(2+i)));   
		} else if ( count < offTime ) { 
			SET_BITS(PTS, (1<<(2+i)));
		} else {						  
			CLEAR_BITS(PTS, (1<<(2+i)));   
		}		
		ledMode[i].count++;	   
		ledMode[i].count %= period;
	}
}