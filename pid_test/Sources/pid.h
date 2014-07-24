/*         PID controller          */
/*   Utilizes the RTI interrupt    */
/* for regular periodic iterations */
#ifndef PID_H
#define PID_H

#include <mc9s12c32.h>
#include "pid.h"

// RTI intervals
// see page 263 of MC9S12 family datasheet for full table of RTI intervals (Table 9-7)
#define RTI_OFF             0x00
#define RTI_64              0x10    // OSCCLK / (1 * 2^10)  = 64us
#define RTI_1024            0x1F    // OSCCLK / (16 * 2^10) = 1024us
#define RTI_2048            0x2F    // OSCCLK / (16 * 2^11) = 2048us
#define RTI_4096            0x3F    // OSCCLK / (16 * 2^12) = 4096us
#define RTI_8192            0x4F    // OSCCLK / (16 * 2^13) = 8192us
#define RTI_16384           0x5F    // OSCCLK / (16 * 2^14) = 16.384ms (61.03Hz)
#define RTI_32768           0x6F    // OSCCLK / (16 * 2^15) = 32.768ms (30.52Hz)
#define RTI_65536           0x7F    // OSCCLK / (16 * 2^16) = 65.536ms (15.26Hz)
#define RTI_MIN             RTI_64
#define RTI_MAX             RTI_65536

#define RTI_INTERVAL        RTI_16384   // Use ~60Hz PID interval

#define PID_SCALING_FACTOR  1000

#define SETPOINT_LIMIT_MAX  1500    // Maximum allowed setpoint value
#define SETPOINT_LIMIT_MIN  50      // Minimum allowed setpoint value


typedef struct {
    int          setpoint;      // Desired output value
    unsigned int Kp;            // Proportional gain
    unsigned int Ki;            // Integral gain
    unsigned int Kd;            // Derivative gain
    int          sp_limit_max, sp_limit_min;    // Setpoint limits (saturation)
    long         int_limit_max, int_limit_min;  // Integral term limits
    long         prev_err;      // Previous error value
    long         int_err;       // Integral error value
    int          feedback;      // Feedback value (sensor measurement)
    long         output;        // Actual output value
} pid_state;

static pid_state volatile pid;  // PID state for use by controller ISR


void pid_init(unsigned int Kp, unsigned int Ki, unsigned int Kd, long limit);
void pid_setpoint(int setpoint);
long pid_output(void);
void pid_reset_integrator(void);

/* Initialize PID controller with gains and integral term limit */
/* NOTE: Must multiply all three gains by PID_SCALING_FACTOR when initializing */
void pid_init(word Kp, word Ki, word Kd, long limit) {
    pid.sp_limit_max = SETPOINT_LIMIT_MAX;
    pid.sp_limit_min = SETPOINT_LIMIT_MIN;
    
    // Setup gains and limits, reset error values
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

/* Return the actual output value */
long pid_output(void) {
    return pid.output;
}

/* Reset integral error term of controller */
void pid_reset_integrator(void) {
    pid.int_err = 0;
}


/* PID controller interrupt */
interrupt VectorNumber_Vrti
void pid_ISR(void) {
    long curr_err;
    long p_term, i_term, d_term;
    
    curr_err = pid.setpoint - pid.feedback;
    
    // Compute proportional term
    p_term = pid.Kp * curr_err;
    
    // Compute integral term and bind to limits
    pid.int_err += curr_err;
    if( pid.int_err > pid.int_limit_max ) {
        pid.int_err = pid.int_limit_max;
    } else if( pid.int_err < pid.int_limit_min ) {
        pid.int_err = pid.int_limit_min;
    }
    i_term = pid.Ki * pid.int_err;
    
    // Compute derivative term from differential error
    d_term = pid.Kd * ( curr_err - pid.prev_err );
    
    
    pid.output = ( p_term + i_term + d_term ) / PID_SCALING_FACTOR; // Summation of all three terms
    pid.prev_err = curr_err;   // Save error term for next iteration
    
    CRGFLG_RTIF = 1;    // Acknowledge RTI (must write 1 to clear flag)
}

#endif // _PID_H