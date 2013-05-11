/*************************************************************************//**
 * \file   twi.h
 * \brief  Low-level TWI function declarations for the ATtiny48.
 * \author Jon Lindsay (Jonathan.Lindsay@asu.edu)
 * \date   20090419 - initial version
 *         20090421 - add TW_STATUS_MASK fix
 ****************************************************************************/

#ifndef TWI_H
#define TWI_H

#include<util/twi.h>
#ifdef __AVR_ATtiny48__
#	undef TW_STATUS_MASK
/// Fix a broken avr-libc header
/** Somebody blew it in avr-libc and defined TWS[3-7] wrong for ATtiny48 only,
 *  making TW_STATUS_MASK 0x7c instead of 0xf8 as it should be. To work around
 *  this, util/twi.h is preemptively included here and TW_STATUS_MASK is
 *  redefined to the correct value.
 */
#	define TW_STATUS_MASK 0xf8
#endif

#include"error.h"

/// Timer function callback type
typedef error_t (*twi_func_t)( char *data, int len );

void twi_init( void );		///<Set up the TWI module for slave operation
void twi_func( twi_func_t );	///<Set function to call when data arrives

#endif
