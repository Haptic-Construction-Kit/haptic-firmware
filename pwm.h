/*************************************************************************//**
 * \file   pwm.h
 * \brief  Low-level pwm function declarations for the ATtiny48.
 * \author Jon Lindsay (Jonathan.Lindsay@asu.edu)
 * \date   20090419 - initial version
 ****************************************************************************/

#ifndef PWM_H
#define PWM_H

void pwm_init( void );		///<Set up the ATtiny's PWM control registers
void pwm_set( int period, int duty ); ///<Configure PWM period and duty cycle
void pwm_on( void );		///<Reset PWM counter and enable PWM
void pwm_off( void );		///<Turn off PWM

#endif
