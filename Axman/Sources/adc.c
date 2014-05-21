#include "adc.h"


void adc_init() {

	ATDCTL2_ADPU = 1;	
    ATDCTL2_AFFC = 0;
    ATDCTL2_AWAI = 0;	 
    ATDCTL2_ETRIGLE = 0;
    ATDCTL2_ETRIGP = 0;
    ATDCTL2_ETRIGE = 0;
    ATDCTL2_ASCIE = 0;
    
    ATDCTL3_S8C = 0;  
    ATDCTL3_S4C = 0;
    ATDCTL3_S2C = 0;
    ATDCTL3_S1C = 1;  
    ATDCTL3_FIFO = 0; 
    ATDCTL3_FRZ1 = 0; 
    ATDCTL3_FRZ0 = 0;
					 
	// 10bit mode
    ATDCTL4_SRES8 = 0;

	// Phase2 sample time
	ATDCTL4_SMP = 0b00;

	// Prescale to 2MHz
	ATDCTL4_PRS = 1;
	




}




word adc_read(byte ch) {


				
	
	// Right-justify
	//ATDCTL5_DJM = 1;
	
	// Unsigned
	//ATDCTL5_DSGN = 0;

	// Single conversion
	//ATDCTL5_SCAN = 0;
	
	// Single channel
	//ATDCTL5_MULT = 0;
	   	
	//ATDCTL5_CC = (ch >> 2) & 0x01; 
	//ATDCTL5_CB = (ch >> 1) & 0x01; 
	//ATDCTL5_CA = (ch >> 0) & 0x01; 
	ATDCTL5 = 0x80 | ch;
	while ( ! ATDSTAT0_SCF );
	return ATDDR0;	
}

