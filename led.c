/*************************************************************************//**
 * \file   led.c
 * \brief  Functions to aid with debugging.
 * \author Jacob Rosenthal (Jacob.Rosenthal@asu.edu)
 * \date   20090714 - initial version
 ****************************************************************************/
#include"led.h"
#include <avr/sfr_defs.h>

//FIXME: tie these to led aliases aliases or alias them
void setup_led( void ){
DDRC |= _BV(DDC1);
DDRC |= _BV(DDC0);
}

//boolean - set light on or off
//port - which port to alter
void set_led(uint8_t port, uint8_t boolean){
if ( boolean )
PORTC|=_BV(port);
else
PORTC&=~_BV(port);
}
