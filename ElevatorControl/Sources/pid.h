#ifndef PID_H
#define PID_H

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

#define SETPOINT_LIMIT_MAX  1450    // Maximum of 1450mm setpoint position
#define SETPOINT_LIMIT_MIN  100     // Minimum of 100mm setpoint position


typedef struct {
    int  setpoint;      // Desired output
    word Kp;            // Proportional gain
    word Ki;            // Integral gain
    word Kd;            // Derivative gain
    int  sp_limit_max, sp_limit_min;    // Setpoint limits (saturation)
    long int_limit_max, int_limit_min;  // Integral term limits
    long prev_err;      // Previous error value
    long int_err;       // Integral error value
    int  feedback;      // Controller feedback (position measurement)
    long output;        // Controller output
} pid_state;

void pid_init(word Kp, word Ki, word Kd, long limit);
void pid_setpoint(int setpoint);
void pid_reset_integrator(void);

#endif
