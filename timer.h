/*************************************************************************//**
 * \file   timer.h
 * \brief  Low-level timer function declarations for the ATtiny48.
 * \author Jon Lindsay (Jonathan.Lindsay@asu.edu)
 * \date   20090421 - initial version
 ****************************************************************************/

#ifndef TIMER_H
#define TIMER_H

#include<inttypes.h>

/// Timer function callback type
typedef void (*timer_func_t)( void );

void timer_init( void );	///<Set up the tiny's timer0 control registers
void timer_set( uint8_t interval );	///<Configure interrupt interval (ms)
void timer_on( void );		///<Reset and enable timer
void timer_off( void );		///<Disable timer
void timer_func( timer_func_t );///<Set function to call when interrupt occurs

#endif
