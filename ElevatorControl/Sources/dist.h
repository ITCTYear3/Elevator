/* HC-SR04 Ultrasonic Sensor */
#ifndef _DIST_H
#define _DIST_H

void dist_init(void);
word dist_read(void);
word get_pulse_overflow_count(void);

#endif // _DIST_H