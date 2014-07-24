/*
 * PID controller test
 */

#include <hidef.h>
#include "derivative.h"
#include "dac.h"
#include "pid.h"

#define K_P     0.3
#define K_I     0.35
#define K_D     0

void main(void) {
    unsigned char a;
    
    SPIinit();
    DACinit();
    pid_init(K_P * PID_SCALING_FACTOR, K_I * PID_SCALING_FACTOR, K_D * PID_SCALING_FACTOR, 1000);
    
    DDRT_DDRT6 = 1;
    PTT_PTT6 = 0;
    
    EnableInterrupts;
    
    pid_setpoint(100);
    
    for(;;) {
        //DACdata((unsigned char)pid_output());
        
        for(a=0; a<255; a++) {
            //a <= 127 ? DACdata(0) : DACdata(255);
            DACdata(a);
        }
        for(a=255-1; a>0; a--) {
            //a <= 127 ? DACdata(0) : DACdata(255);
            DACdata(a);
        }
    }
}
