/*************************************************************************//**
 * \file   twi.c
 * \brief  Low-level TWI functions for the ATtiny48.
 * \author Jon Lindsay (Jonathan.Lindsay@asu.edu)
 * \date   20090714 - last edit jjrosent 
 ****************************************************************************/

#include<stdlib.h>	// for NULL
#include<avr/interrupt.h>
#include<util/twi.h>

#include"learn_tiny.h"
#include"twi.h"
#include"error.h"
#include"led.h"
#include"globals.h"
#include <avr/eeprom.h> 

//#include"debug.h"

uint8_t EEMEM TWI_ADDRESS;

/// Input port mask for the TWI slave address switchbank
#define TWI_ADDR_MASK ( _BV(PORTD5) | _BV(PORTD4) | _BV(PORTD3) | \
		_BV(PORTD2) | _BV(PORTD1) | _BV(PORTD0) )

/// Function to be called when data has been completely received over TWI
static twi_func_t twi_callback = NULL;

void twi_error ( void ) {
	set_led(ELED,1);
}

/// Interrupt handler for all things TWI
ISR( TWI_vect )
{
	static char buf[ PARSE_MAX_LEN ];
	static error_t status = ESUCCESS;
	static int ind = 0;
	static int outInd = 0;
	
	switch( TW_STATUS ) {
	case TW_BUS_ERROR:
		TWCR |= _BV( TWSTO );
		twi_error();
		break;	// FIXME: error led
	case TW_SR_SLA_ACK: //SLA+W received, ACK returned
	case TW_SR_GCALL_ACK:	// FIXME? do something different for GCALL?
		ind = 0;
		break;
	case TW_SR_DATA_ACK: //data received, ACK returned
	case TW_SR_GCALL_DATA_ACK: // FIXME? something different for GCALL?
		if( ind < sizeof(buf) )
			buf[ ind++ ] = TWDR;
		break;
	case TW_SR_STOP: //stop or repeated start condition received while selected
		// arduino Wire library doesn't allow repeated start...
		// still okay to let the funnel release the bus before sending
		// a new start for the response, because it is the only master

		if( twi_callback == NULL )
			status = EMISSING;
		else if( ind >= sizeof(buf) )
			status = ETOOBIG;
		else {
			buf[ ind ] = '\0';
			status = twi_callback( buf, ind );
		}
		break;
	case TW_ST_SLA_ACK: ///SLA+R received, ACK returned
		TWDR = status;
		break;
	case TW_ST_DATA_ACK: //data transmitted, ACK received
		//increment pointer or wrap around, 
		//if you asked for more bytes than your command, its your fault
		//optionally we can keep track of how many bytes we have to send and use TWEA
		if(outInd > OUT_MAX_LEN - 1)
			outInd=0;
		TWDR = glbl.bufOut[ outInd++ ];
		break;
	case TW_ST_DATA_NACK: ///data transmitted, NACK received //arduino transmits nack on last byte..pg151?
	case TW_ST_LAST_DATA:  //last data byte transmitted, ACK received
		//reset outbound pointer - does this get hit?
		outInd=0;
		// set TWEA again to respond when addressed
		TWCR = (TWCR & ~_BV( TWINT )) | _BV( TWEA );
		break;
	default:	// FIXME: error led
//		dumpbyte(TW_STATUS);
		twi_error();
		break;
	}

	// clear the interrupt flag to allow the TWI module to continue
	TWCR |= _BV( TWINT );
}

error_t check_twi_address(uint8_t twi_address){
	if( twi_address > 7 && twi_address < 120 )
	return ESUCCESS;
	else
	return EARG;
	}

error_t set_twi_address(uint8_t twi_address){
	error_t status = check_twi_address( twi_address );
	
	if( status == ESUCCESS ){
		eeprom_busy_wait();
		eeprom_update_byte( &TWI_ADDRESS, twi_address );
	}
	return status;
}

void twi_init( void )
{
	// enable the TWI clock
	PRR &= ~_BV( PRTWI );

	// configure the low PORTD pins as inputs and enable pullups
	// must do this before configuring PORTC, so that the port is ready
	// for reading by the time we set TWAR
	PORTD |= TWI_ADDR_MASK;
	DDRC &=~(_BV(DDD5)|_BV(DDD4)|_BV(DDD3)|_BV(DDD2)|_BV(DDD1)|_BV(DDD0));

	// set SCL and SDA pins to inputs with internal pullups enabled
	PORTC |= _BV( PORTC4 ) | _BV( PORTC5 );
	DDRC &= ~( _BV(DDC4) | _BV(DDC5) );

	//if eeprom has address, use it, otherwise set slave address to default -- enable the general call
	eeprom_busy_wait();
	uint8_t address = eeprom_read_byte(&TWI_ADDRESS);
	if( check_twi_address(address) == ESUCCESS )
		TWAR = (address << 1) | _BV( TWGCE );
	else
		TWAR = (DEFAULT_TWI_ADDRESS << 1) | _BV( TWGCE );
		
	// configure the TWI module
	TWCR =	_BV( TWEA ) |	// ack when addressed or data received
		_BV( TWEN ) |	// enable TWI
		_BV( TWIE );	// enable the TWI interrupt

	// no prescaler necessary, since master mode is never used
}

void twi_func( twi_func_t func )
{
	twi_callback = func;
}