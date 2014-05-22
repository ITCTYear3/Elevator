/*
 * CAN Bus test
 *
 */

#include <hidef.h>
#include "derivative.h"
#include "utils.h"
#include "timer.h"  // for msleep()
#include "mscan.h"


void main(void) {
    dword id = 0x100;
    byte priority = 0x01;
    byte data[] = "TEST";
    byte ret;
    
    timer_init();
    CANinit();
    
    // Clear all receiver flags (by setting bits)
    CANRFLG = (CANRFLG_RXF_MASK | CANRFLG_OVRIF_MASK | CANRFLG_CSCIF_MASK | CANRFLG_WUPIF_MASK);
    
    EnableInterrupts;
    
    for(;;) {
        ret = CANsend(id, priority, sizeof(data)-1, data);
        if(ret) {
            // Message could not be sent!
        }
        msleep(1000);   // Wait a bit before sending another
    }
}
