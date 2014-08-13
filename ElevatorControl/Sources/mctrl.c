/*
 * mctrl.c
 * Elevator motor control for servo module
 * PID controller takes a setpoint input and outputs a value to the DAC
 * Utilizes DAC for analog differential voltage to servo
 */

#include <mc9s12c32.h>
#include "mctrl.h"
#include "dac_max553x.h"   // 12-bit DAC
#include "spi.h"
#include "pid.h"

#define K_P     0.3
#define K_I     0
#define K_D     0


void mctrl_init(void) {
    int const integral_limit = 1000;
    
    SPIinit();
    DACinit();
    pid_init(K_P * PID_SCALING_FACTOR, K_I * PID_SCALING_FACTOR, K_D * PID_SCALING_FACTOR, integral_limit, 0.25);
    
}

/* Update analog differential voltage output to servo */
void mctrl_update(void) {
    int data = pid_output() & 0x0FFF;
    
    DACpreloadA(data);                  // Preload data into DAC channel A internal register
    DACloadBshiftA( (4096 - data) );    // Load data into DAC channel B and output it; load in preloaded channel A data and output it
    
}
