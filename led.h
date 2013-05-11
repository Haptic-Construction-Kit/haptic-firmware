/*************************************************************************//**
 * \file   led.h
 * \brief  Function declarations to aid with debugging.
 * \author Jacob Rosenthal (Jacob.Rosenthal@asu.edu)
 * \date   20090714 - initial version
 ****************************************************************************/

#ifndef LED_H
#define LED_H

#include<avr/io.h>
#define ELED PORTC1 
#define SLED PORTC0 

void setup_led( void );	///<Engage or disengage a light  
void set_led(uint8_t port, uint8_t boolean); ///<Engage or disengage a light

#endif
