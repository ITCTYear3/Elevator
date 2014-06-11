/* Timer module macros */
#ifndef _TIMER_H
#define _TIMER_H

#include "utils.h"


#define TC_SLEEP        3   // msleep function timer channel (0-7), must not conflict with other channels in use
                            // (suggest using TC3, PT3 is unconnected on controller board)

// Timer modes: timer enable, fast flag clear enable, counter stop while in freeze mode
#define TSCR1_INIT      ( TSCR1_TEN_MASK | TSCR1_TFFCA_MASK | TSCR1_TSFRZ_MASK )

#define FAST_FLAG_CLR       // Fast flag clear is enabled

// Output compare TCNT deltas
#define OC_DELTA_1MS        1000                    // 1ms delta with prescaler of 8
#define OC_DELTA_1US        1                       // 1us delta with prescaler of 8
#define TIMER_DELTA(ms)     ((ms) * OC_DELTA_1MS)   // Delta to set up timer channel for next event

// Output compare duty times
#define TIMER_1_PERIOD      500     // period in ms
#define TIMER_1_DUTY_CYCLE  20      // % Duty cycle
#define TIMER_1_ON_TIME     (((TIMER_1_PERIOD * TIMER_1_DUTY_CYCLE) / 100) * OC_DELTA_1MS)
#define TIMER_1_OFF_TIME    ((TIMER_1_PERIOD * OC_DELTA_1MS) - TIMER_1_ON_TIME)

/*****************************************************************************/

#define EnableTimer     SET_BITS(TSCR1,TSCR1_TEN_MASK)
#define DisableTimer    CLR_BITS(TSCR1,TSCR1_TEN_MASK)

#define TOI_ENABLE      SET_BITS(TSCR2,TSCR2_TOI_MASK)  // Enable timer overflow interrupt
#define TOI_DISABLE     CLR_BITS(TSCR2,TSCR2_TOI_MASK)  // Disable timer overflow interrupt
#define TC(chan)        CAT(TC,chan)                    // Select timer channel
#define TC_OC(chan)     SET_BITS(TIOS, 1 << (chan))     // Set channel to output compare
#define TC_IC(chan)     CLR_BITS(TIOS, 1 << (chan))     // Set channel to input capture


// Macro to calculate the vector # given the vector address
// - take 1's complement of address to get a positive value
// - mask off lower byte since that's the only meaningful data - rest should be 0
// - right shift by 1 (divide by 2) to get the vector number since each vector address is 2 bytes wide
// - (The LSB will be 1 before shift.  This will get shifted off on the divide by 2.  e.g. 3/2 = 1, 5/2= 2)
#define VECTOR_NUM(vector_addr)     (((~(vector_addr)) & 0xFF) >> 1)
#define TC_VECTOR(chan)             CAT(Vtimch,chan)    // Build timer channel vector name from channel number


// Timer prescale macros to set one of 8 possible prescale values
#define SET_TCNT_PRESCALE(scale)    FORCE_BITS(TSCR2,TSCR2_PR_MASK,(scale))
#define TCNT_PRESCALE_1             0x00  // TCNT at 8MHz
#define TCNT_PRESCALE_2             0x01  // TCNT at 4MHz
#define TCNT_PRESCALE_4             0x02  // TCNT at 2MHz               
#define TCNT_PRESCALE_8             0x03  // TCNT at 1MHz
#define TCNT_PRESCALE_16            0x04  // TCNT at 500kHz
#define TCNT_PRESCALE_32            0x05  // TCNT at 250kHz
#define TCNT_PRESCALE_64            0x06  // TCNT at 125kHz
#define TCNT_PRESCALE_128           0x07  // TCNT at 62.5kHz


// Timer interrupt macros
#define TC_INT_ENABLE(chan)     SET_BITS(TIE, 1 << (chan))  // Enable timer module interrupts from channel
#define TC_INT_DISABLE(chan)    CLR_BITS(TIE, 1 << (chan))  // Disable timer module interrupts from channel
#define TC_INT_TRIG(chan)       ((TFLG1 >> (chan)) & 0x01)  // True if timer module interrupt has occurred


// Output compare result output actions
// 0x03 in macro is the mask for the action value (2 bits)
#define TCTL_1_2            (*(volatile word * const) & TCTL1)  // Name definition to access the timer control registers 1 and 2 as a 16-bit word
#define SET_OC_ACTION(chan,action)  FORCE_WORD(TCTL_1_2,(0x03 << ((chan) * 2)),((action) << ((chan) * 2)))
#define FORCE_OC_ACTION(chan)       SET_BITS(CFORC,(0x01 << (chan)))    // Macro to cause the OC action specifed for a channel to happen immediately
#define OC_OFF              0x00
#define OC_TOGGLE           0x01
#define OC_FLIP             0x01
#define OC_LO               0x02
#define OC_HI               0x03

// Input capture edge detect config
// 0x03 in macro is the mask for the action value (2 bits)
#define TCTL_3_4            (*(volatile word * const) & TCTL3)  // Name definition to access the timer control registers 3 and 4 as a 16-bit word
#define SET_IC_EDGE(chan,edge)      FORCE_WORD(TCTL_3_4,(0x03 << ((chan) * 2)),((edge) << ((chan) * 2)))
#define IC_OFF              0x00 
#define IC_DETECT_RISING    0x01
#define IC_DETECT_FALLING   0x02
#define IC_DETECT_BOTH      0x03
#define IC_DETECT_ANY       0x03

/*****************************************************************************/

void timer_init(void);
word get_overflow_count(void);
void msleep(word);


#endif // _TIMER_H