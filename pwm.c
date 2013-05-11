/*************************************************************************//**
 * \file   pwm.c
 * \brief  Low-level pwm functions for the ATtiny48.
 * \author Jon Lindsay (Jonathan.Lindsay@asu.edu)
 * \date   20090429 - initial version
 ****************************************************************************/

#include<avr/io.h>

#include"pwm.h"

#define PWMCS_MASK (_BV(CS02) | _BV(CS01) | _BV(CS00))	///<Clock source mask
#define PWMCS_1024 ( _BV(CS12) | _BV(CS10) )	///<Clock divider = 1024
#define PWMCS_256 ( _BV(CS12) )			///<Clock divider = 256
#define PWMCS_64 ( _BV(CS11) | _BV(CS10) )	///<Clock divider = 64
#define PWMCS_8 ( _BV(CS11) )			///<Clock divider = 8
#define PWMCS_1 ( _BV(CS10) )			///<No clock divider

void pwm_init( void )
{
	// enable the PWM/counter module
	PRR &= ~_BV( PRTIM1 );

	// clear OC1B before enabling the pin as output (see datasheet 13.7.3)
	TCCR1A |= _BV( COM1B1 );	// clear on match, set at TOP
	TCCR1C |= _BV( FOC1B );		// force a match on OC1B
	TCCR1C &= ~_BV( FOC1B );	// this shouldn't do anything...
	TCCR1A &= ~_BV( COM1B1 );	// disable waveform generator output

	// set counter 1 for fast PWM mode (TOP in ICR1)
	// ICR1 controls period, OCR1B duty cycle, output on OC1B
	TCCR1A = _BV( WGM11 );
	TCCR1B = _BV( WGM13 ) | _BV( WGM12 );
	// clock source not set (PWM disabled) until pwm_on

	// configure OC1B pin as output and set it to zero
	// this outputs the PORTB value, not the OC1B value, until pwm_on
	PORTB &= ~2;
	DDRB |= _BV( DDB2 );
	PORTB &= ~2;
}

void pwm_on( void )
{
	// enable the PWM clock with no divider (counts microseconds)
	TCCR1B |= PWMCS_1;

	// switch output from PORTB value to OC1B (waveform generator)
	// OC1B: clear on match, set at TOP
	TCCR1A |= _BV( COM1B1 );
}

void pwm_off( void )
{
	// set counter to TOP - 1 to ensure that the first PWM cycle sets
	// OC1B, gets a full period, and resets the counter in case pwm_set()
	// was called with a shorter period since the last PWM cycle
	// moved here from pwm_on() so that output is not affected by multiple
	// calls to pwm_on() 
	TCNT1 = ICR1 - 1;

	// OC1B: normal pin operation (output PORTB value, not OC1B value)
	TCCR1A &= ~_BV( COM1B1 );

	// disable PWM clock
	TCCR1B &= ~PWMCS_MASK;
}

void pwm_set( int period, int duty )
{
	ICR1 = period;	// occurs immediately since ICR1 is not double-buffered
	OCR1B = duty;
}
