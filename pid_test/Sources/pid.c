/*
 * pid.c
 * PID controller
 * Utilizes the RTI interrupt for regular periodic iterations
 */

#include <mc9s12c32.h>
#include "pid.h"


/* Initialize PID controller with gains and integral term limit */
/* NOTE: Must multiply all three gains by PID_SCALING_FACTOR when initializing */
void pid_init(int Kp, int Ki, int Kd, long limit) {
    
    // Setup gains and limits
    pid.Kp = Kp;
    pid.Ki = Ki;
    pid.Kd = Kd;
    pid.sp_limit_max = 1500;
    pid.sp_limit_min = 50;
    pid.out_limit_max = pid.sp_limit_max;
    pid.out_limit_min = pid.sp_limit_min;
    pid.int_limit = limit;
    
    // Reset error values
    pid.prev_err = 0;
    pid.int_err = 0;
    
    RTICTL = RTI_INTERVAL;
    CRGFLG_RTIF = 1;    // Clear any pending interrupts
    CRGINT_RTIE = 1;    // Enable RTI
}

/* Change the controller setpoint */
void pid_setpoint(int setpoint) {
    if( setpoint > pid.sp_limit_max ) {
        setpoint = pid.sp_limit_max;
    } else if( setpoint < pid.sp_limit_min ) {
        setpoint = pid.sp_limit_min;
    }
    
    pid.setpoint = setpoint;
}

/* Return the controller output value */
int pid_output(void) {
    return pid.output;
}

/* Reset integral error term of controller */
void pid_reset_integrator(void) {
    pid.int_err = 0;
}


/* PID controller interrupt */
interrupt VectorNumber_Vrti
void pid_ISR(void) {
    long err;
    long p_term, i_term, d_term;
    
    err = pid.setpoint - pid.feedback;
    
    // Compute proportional term
    p_term = pid.Kp * err;
    
    // Compute integral term and bind to saturation limits
    pid.int_err += pid.Ki * err;
    if( pid.int_err > pid.int_limit ) {
        pid.int_err = pid.int_limit;
    } else if( pid.int_err < -pid.int_limit ) {
        pid.int_err = -pid.int_limit;
    }
    i_term = pid.int_err;
    
    // Compute derivative term from differential error
    d_term = pid.Kd * ( err - pid.prev_err );
    
    // Compute final output value and bind to saturation limits
    pid.output = ( p_term + i_term + d_term ) / PID_SCALING_FACTOR;
    if( pid.output > pid.out_limit_max ) {
        pid.output = pid.out_limit_max;
    } else if( pid.output < pid.out_limit_min ) {
        pid.output = pid.out_limit_min;
    }
    
    pid.prev_err = err;   // Save error term for next iteration
    
    CRGFLG_RTIF = 1;    // Acknowledge RTI (must write 1 to clear flag)
}
