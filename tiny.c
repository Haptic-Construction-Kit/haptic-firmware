/*************************************************************************//**
 * \file   tiny.c
 * \brief  Main code for the ATtiny48.
 * \author Jon Lindsay (Jonathan.Lindsay@asu.edu)
 * \date   20090714 - last edit jjrosent 
 ****************************************************************************/

#include<string.h>
#include<avr/interrupt.h>
#include<avr/power.h>
#include<avr/sleep.h>

#include"globals.h"
#include"twi.h"
#include"pwm.h"
#include"timer.h"
#include"rhythm.h"
#include"magnitude.h"
#include"vibration.h"
#include"error.h"
#include"learn_tiny.h"
#include"led.h"

//#include"debug.h"

/// ATtiny globals, loaded at startup via LRN commands from the Funnel I/O
globals_t glbl;

/// Called every 50ms to handle the next step of the active rhythm
/** test */
void rhythm_step( void )
{
	rhythm_t *r = glbl.rhythms + glbl.ar;

	// if the current rhythm has finished, just return
	if( glbl.ac < 1 ) {
		pwm_off();
		// FIXME: go to sleep to wait for next command
		return;
	}

	// activate the motor if the current pattern bit is set
	if( r->pattern[glbl.ab/8] & ((uint8_t)0x80>>(glbl.ab%8)) )
		pwm_on();
	else
		pwm_off();

	// move to the next bit to prepare for next call to this function
	++glbl.ab;
	if( glbl.ab >= r->bits ) {
		// current rhythm cycle is complete
		// decrement remaining cycles and reset to start of pattern
		// run indefinitely if we got a maximum cycle count
		if( glbl.ac < MAX_DURATION )
			--glbl.ac;
		glbl.ab = 0;
	}
}

/// Set up the globals for playback of a newly specified rhythm
void vibrate( vibration_t v )
{
	magnitude_t *m = glbl.magnitudes + v.magnitude;

	// adjust PWM settings for the new magnitude
	// FIXME? reset timer instead of killing old rhythm now and waiting
	// until the next 50ms chunk to start the new one?
	pwm_off();
	pwm_set( m->period, m->duty );

	// initialize the global rhythm settings
	glbl.ar = v.rhythm;
	glbl.ab = 0;
	glbl.ac = v.duration;
}

/// Parser for activate mode commands
error_t handle_operate( vibration_t cmd )
{
	if( !cmd.duration )
		;	// stop motor, even if rhythm/mag invalid
	else if( !glbl.rhythms[cmd.rhythm].bits )
		return ENOR;
	else if( !glbl.magnitudes[cmd.magnitude].period )
		return ENOM;

	vibrate( cmd );

	return ESUCCESS;
}

/// Handle a command received from the main controller over TWI (callback)
error_t receive_command( char *cmd, int len )
{
	if( len == sizeof(vibration_t) )
		return handle_operate( *((vibration_t*)cmd) );

	return handle_learn( cmd );
}

/// Top-level ATtiny initialization. Set up PWM, TWI, and timers; loop forever
int main( void )
{
	// make sure all rhythms/magnitudes are initially undefined
	memset( &glbl, 0, sizeof(glbl) );

	// enable the pwm module
	pwm_init();

	// register the TWI data handler and enable TWI
	twi_func( receive_command );
	twi_init();

	// configure the timer to call rhythm_step() every 50ms
	timer_init();
	timer_set(50);
	timer_func( rhythm_step );

	// start the timer
	timer_on();

	// enable interrupts globally
	sei();
	
	setup_led();
	//set_led(SLED,1);

	//power down stuff we're not using
	//we have PRR, but attiny48/88 not listed in the avr docs?
	//PRR has twi tim0 tim1 spi and adc
	power_spi_disable();
	power_adc_disable();

	// from here on out everything happens via interrupts
	// go to sleep, princess
	while(1){
		sleep_enable();//enable sleep
		sleep_bod_disable();//brown out disable
		sleep_cpu(); //sleep
		sleep_disable();//wakeup
	}	

	return 0;
}
