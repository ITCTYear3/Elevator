/*
 * mctrl.h
 * Elevator motor control for servo module
 * Utilizes DAC for analog differential voltage to servo
 */
#ifndef _MCTRL_H
#define _MCTRL_H


void mctrl_init(void);
void mctrl_update(void);

#endif // _MCTRL_H
