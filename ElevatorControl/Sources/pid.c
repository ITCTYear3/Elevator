/* PID controller test */

#include <mc9s12c32.h>
#include "pid.h"

static pid_state volatile pid;    // PID state for use by controller ISR


/* Initialize PID controller with gains and integral term limit */
void pid_init(word Kp, word Ki, word Kd, long limit) {
    pid.sp_limit_max = SETPOINT_LIMIT_MAX;
    pid.sp_limit_min = SETPOINT_LIMIT_MIN;
    
    pid.Kp = Kp;
    pid.Ki = Ki;
    pid.Kd = Kd;
    pid.int_limit_max = limit;
    pid.int_limit_min = -limit;
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

/* Reset integral error term of controller */
void pid_reset_integrator(void) {
    pid.int_err = 0;
}


interrupt VectorNumber_Vrti
void pid_ISR(void) {
    long curr_err;
    long p_term, i_term, d_term;
    
    curr_err = pid.setpoint - pid.feedback;
    
    // Compute proportional term
    p_term = pid.Kp * curr_err;
    
    // Compute integral term and bind to limits
    pid.int_err += curr_err;
    if( pid.int_err < -(pid.int_limit) ) {
        pid.int_err = -(pid.int_limit);
    } else if( pid->int_err > pid.int_limit ) {
        pid.int_err = pid.int_limit;
    }
    i_term = pid.Ki * pid.int_err;
    
    // Compute derivative term from differential error
    d_term = pid.Kd * ( curr_err - pid.prev_err );
    
    
    pid.output = p_term + i_term + d_term; // Summation of all three terms
    pid.prev_err = curr_err;   // Save error term for next iteration
    
    CRGFLG_RTIF = 1;    // Acknowledge RTI (must write 1 to clear flag)
}
