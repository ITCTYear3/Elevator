/* PID controller test */

#include <hidef.h>      // For EnableInterrupts and DisableInterrupts macros
#include <mc9s12c32.h>

// RTI intervals
// see page 263 of MC9S12 family datasheet for full table of RTI intervals
#define RTI_INTERVAL        RTI_MAX
#define RTI_OFF             0x00
#define RTI_MAX             0x7F        // OSCCLK / (16 * 2^16)


typedef struct {
    word setpoint;      // Desired output
    word Kp;            // Proportional gain
    word Ki;            // Integral gain
    word Kd;            // Derivative gain
    long int_limit;     // Integral term limiter (-limit to +limit)
    long prev_err;      // Previous error
    long int_err;       // Integral error
    word input;         // Controller input (from sensor)
    word output;        // Controller output
} PID;

static PID volatile pid;    // PID state for use by controller ISR


void PID_init(word Kp, word Ki, word Kd) {
    pid.Kp = Kp;
    pid.Ki = Ki;
    pid.Kd = Kd;
    pid.prev_err = 0;
    pid.int_err = 0;
    pid.diff_err = 0;
    
    RTICTL = RTI_INTERVAL;
    CRGFLG_RTIF = 1;    // Clear any pending interrupts
    CRGINT_RTIE = 1;    // Enable RTI
}

interrupt VectorNumber_Vrti
void PID_ISR(void) {
    long curr_err, diff_err;
    long p_term, i_term, d_term;
    
    curr_err = pid.setpoint - pid.input;
    
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
    diff_err = curr_err - pid.prev_err;
    d_term = pid.Kd * diff_err;
    
    
    pid.output = p_term + i_term + d_term; // Summation of all three terms
    pid.prev_err = curr_err;   // Save error term for next iteration
    
    CRGFLG_RTIF = 1;    // Acknowledge RTI (must write 1 to clear flag)
}
