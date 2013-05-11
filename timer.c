/*************************************************************************//**
 * \file   timer.c
 * \brief  Low-level timer functions for the ATtiny48.
 * \author Jon Lindsay (Jonathan.Lindsay@asu.edu)
 * \date   20090418 - initial version
 ****************************************************************************/

#include<avr/interrupt.h>
#include<stdlib.h>	// for NULL

#include"timer.h"

#define TCS_MASK ( _BV(CS02) | _BV(CS01) | _BV(CS00) )	///<Clock source mask
#define TCS_1024 ( _BV(CS02) | _BV(CS00) )	///<Clock divider = 1024
#define TCS_256 ( _BV(CS02) )			///<Clock divider = 256
#define TCS_64 ( _BV(CS01) | _BV(CS00) )	///<Clock divider = 64
#define TCS_8 ( _BV(CS01) )			///<Clock divider = 8
#define TCS_1 ( _BV(CS00) )			///<No clock divider

/// Function to call when a timer interrupt occurs
static timer_func_t timer_callback = NULL;

/// Actual interrupt handler for the timer interrupt
ISR( TIMER0_COMPA_vect )
{
	if( timer_callback != NULL )
		timer_callback();
}

void timer_init( void )
{
	// enable the timer0 module
	PRR &= ~_BV( PRTIM0 );

	// set timer0 for clear on compare match mode
	// OCR0A controls TOP (interval between timer interrupts)
	// clock source not set until timer_on()
	TCCR0A = _BV( CTC0 );	// CTC mode, TOP in OCR0A

	// enable interrupt on compare match A (TCNT == ORC0A)
	// leave interrupts for overflow and compare match B disabled
	TIMSK0 = _BV( OCIE0A );

	// no output pin for the counter since it only generates interrupts
}

void timer_set( uint8_t interval )
{
	// timer0 will generate an interrupt when TCNT0 reaches this value
	OCR0A = interval;
}

void timer_func( timer_func_t func )
{
	timer_callback = func;
}

void timer_on( void )
{
	// clear the counter so the first interrupt doesn't happen early
	TCNT0 = 0;

	// set timer0 clock source to enable the counter
	// assumes that the clock select bits are cleared (clock disabled)
	// or already set to the same value being written here
	TCCR0A |= TCS_1024;
}

void timer_off( void )
{
	// disable the timer0 clock
	TCCR0A &= ~TCS_MASK;
}
