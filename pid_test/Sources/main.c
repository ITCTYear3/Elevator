/*
 * PID controller test
 */

#include <hidef.h>
#include "derivative.h"
#include "dac_max551x.h"    // 8-bit DAC
//#include "dac_max553x.h"   // 12-bit DAC
#include "spi.h"
#include "pid.h"

#define K_P     0.3
#define K_I     0.35
#define K_D     0

#define SAWTOOTH
//#define SQUARE

void main(void) {
    unsigned char a;
    
    SPIinit();
    DACinit();
    pid_init(K_P * PID_SCALING_FACTOR, K_I * PID_SCALING_FACTOR, K_D * PID_SCALING_FACTOR, 1000);
    
    EnableInterrupts;
    
    pid_setpoint(100);
    
    for(;;) {
        //DACdata((unsigned char)pid_output());
        
        for(a=0; a<255; a++) {
#ifdef SAWTOOTH
            DACdata(a);
#else
#ifdef SQUARE
            a <= 127 ? DACdata(0) : DACdata(255);
#endif
#endif
        }
        for(a=255-1; a>0; a--) {
#ifdef SAWTOOTH
            DACdata(a);
#else
#ifdef SQUARE
            a <= 127 ? DACdata(0) : DACdata(255);
#endif
#endif
        }
    }
}
