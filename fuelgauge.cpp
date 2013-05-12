/*************************************************************************//**
 * \file   fuelgauge.cpp
 * \brief  Functions for communication with the MAXIM DS2782 fuel gauge IC.
 * \author Jon Lindsay (Jonathan.Lindsay@asu.edu)
 * \date   20091025 - initial version
 ****************************************************************************/

#include <Wire.h>

#include "fuelgauge.h"
#include "wire_err.h"

#include "debug_main.h"

#define FG_DEF_ADDR 0x34	///< DS2782 factory default slave address
#define FGR_FUNC 0xfe	///< Function command register address, for fg_func()

/// Maximum time to wait for a response from the DS2782, in ms
#define TWI_TIMEOUT 100

/// Actual address to use for communication with DS2782
static uint8_t fg_addr = FG_DEF_ADDR;

// and arduino sucks in another way--why is millis() automatically available
// in the main sketch file without #including anything, but not here?
unsigned long mswrap();

/// Temporary type for fg_get() / fg_set()
typedef union {
	uint8_t u1;
	uint16_t u2;
	int8_t s1;
	int16_t s2;
} register_storage_t;

/// Convert Wire.endTransmission() return value into an error_t status
/** \param wire_return
 *	Return value from a call to Wire.endTransmission().
 *
 *  \retval ESUCCESS
 *	Data was transmitted successfully.
 *  \retval EBUS, EBUSAN, EBUSDN
 *	Communication with the DS2782 failed. See error_t for details.
 *  \retval EBUSOF Hit an(other) Arduino bug.
 */
static error_t wire_status( uint8_t wire_return )
{
	// if the TWI transmission failed, return a specific bus error status
	if( wire_return == WE_SUCCESS )
		return ESUCCESS;

	if( wire_return >= WE_ERROR )
		return EBUS;
	else	return (error_t)(EBUS + wire_return);
}

/** Probe for the DS2782 at \a addr. If there is no response at that address,
 *  probe the DS2782 factory address (FG_DEF_ADDR). If the device responds at
 *  the factory address, assume it is fresh from the factory and program belt
 *  parameters onto the device (remap the slave address to \a addr, set
 *  application parameters, and store the changes in device EEPROM).
 *
 *  \retval ESUCCESS
 *	The DS2782 was found and (if necessary) successfully programmed.
 *  \retval ENOMOTOR
 *	No response from the DS2782 at either \a addr or FG_DEF_ADDR.
 *  \retval x
 *	Where \a x is any error code that can be returned by fg_get() or
 *	fg_set().
 */
error_t fg_init( uint8_t addr )
{
	error_t status;

	DBGN( "fg" );
	DBGN( addr, HEX );
	fg_addr = addr;
	if( wire_status(FGG_STATUS(fg_get(FGR_STAT))) == ESUCCESS )
		return ESUCCESS;	// present and initialized

	DBGN( FG_DEF_ADDR, HEX );
	fg_addr = FG_DEF_ADDR;
	if( wire_status(FGG_STATUS(fg_get(FGR_STAT))) != ESUCCESS )
		return ENOMOTOR;	// not found at either address

	// present, but in factory default state--set application parameters
	DBGN( "rm" );
#define TRY( func ) if( (status=func) != ESUCCESS ) return status
	TRY( fg_set(FGR_ADDR, addr) );
	fg_addr = addr;
	TRY( fg_set(FGR_RSP, 50) );	// mhos
	TRY( fg_set(FGR_VCHG, 210) );	// * 19.52 mV = 4.1 V
	TRY( fg_set(FGR_IMIN, 20) );	// * 50 uV * 50 mhos = 50 mA
	TRY( fg_set(FGR_VAE, 150) );	// * 19.52 mV = 2.9128 V
	TRY( fg_set(FGR_IAE, 100) );	// * 200 uV * 50 mhos = 1 A
	TRY( fg_set(FGR_AC, 2240) );	// * 6.25 uVh * 50 mhos = 700 mAh
	TRY( fg_set(FGR_AS, 121) );	// * 0.78125% = 95%

	// store the parameters in EEPROM
	TRY( fg_func(FGF_SAVE_1) );
#undef TRY

	DBGN( "ok" );
	return ESUCCESS;
}

/** Execute the given function on the DS2782. See fg_func_t for brief
 *  descriptions of the supported commands. Details can be found in the DS2782
 *  datasheet.
 *
 *  \param func
 *	The function to execute.
 *
 *  \retval ESUCCESS
 *	Communication with the DS2782 succeeded.
 *  \retval EARG
 *	\a func is not a valid DS2782 function.
 *  \retval x
 *	Where \a x is any error code that can be returned by wire_status().
 */
error_t fg_func( fg_func_t func )
{
	uint8_t status;

	switch( func ) {
	case FGF_SAVE_0:
	case FGF_SAVE_1:
	case FGF_LOAD_0:
	case FGF_LOAD_1:
	case FGF_LOCK_0:
	case FGF_LOCK_1:	break;
	default:		return EARG;
	}

	Wire.beginTransmission( fg_addr );
	Wire.write( (uint8_t)FGR_FUNC );
	Wire.write( (uint8_t)func );
	status = Wire.endTransmission();

	return wire_status( status );
}

/** Read one or more registers from the DS2782 into memory pointed at by
 *  \a buf. Raw register data is stored without any data conversion. See
 *  fg_get() for a more convenient interface.
 *
 *  If \a size specifies more data to read than is occupied by the register at
 *  \a addr, the additional data comes from the following register(s), in the
 *  order listed in fg_register_t. Data returned from reserved addresses is
 *  undefined. See page 26 of the DS2782 datasheet for more details.
 *
 *  \param addr
 *	The address to start reading at. Any address is allowed, but probably
 *	only those defined in fg_register_t make sense.
 *  \param buf
 *	Destination buffer. Must be large enough to hold \a size bytes.
 *  \param size
 *  	Number of bytes (not the number of registers) to read.
 *
 *  \retval ESUCCESS
 *	All requested data was successfully read.
 *  \retval x
 *	Where \a x is any error code that can be returned by wire_status().
 */
error_t fg_get_multi( uint8_t addr, void *buf, uint8_t size )
{
	// luckily the fuel gauge does not appear to require repeated start,
	// even though the datasheet implies that it does
	unsigned long start;
	error_t status;
	uint8_t *into = (uint8_t*)buf;

	Wire.beginTransmission( fg_addr );
	Wire.write( addr );
	status = wire_status( Wire.endTransmission() );
	if( status != ESUCCESS )
		return status;

	// wait for at most TWI_TIMEOUT milliseconds
	for( start=mswrap(); mswrap()-start < TWI_TIMEOUT; ) {
		if( Wire.requestFrom(fg_addr, size) ) {
			// uh. what if TWI receive fails?
			// FIXME: this is probably wrong. need to consult
			// Wire documentation for requestFrom() and receive().
			while( size-- > 0 )
				*into++ = Wire.read();

			return ESUCCESS;
		}
	}

	return EBUS;
}

/// Extract the register data mask from an action
#define ACT_MASK( action ) ((action) >> 16)
/// Extract the register shift amount from an action
#define ACT_SAMT( action ) (((action)>>8) & 0xf)
/// Extract the register size from an action
#define ACT_SIZE( action ) ((action) & 0xf)
/// Determine whether a select_action() return value represents an error
#define ACT_FAIL( action ) (ACT_MASK(action) == 0)
/// Extract the error code from a select_action() error return value
#define ACT_ERRC( action ) ((error_t)action)

/// Decide how fg_get() / fg_set() should handle the selected register data
static uint32_t select_action( fg_register_t addr )
{
	// action format: 0xmmmm0Ssb
	// m = mask, S = shift amount, s = signed/unsigned, b = byte count
	// careful, if the sign bit of a signed value is not the MSb of its
	// register then the mask code in fg_get() has to change!
	switch( addr ) {
	case FGR_STAT:	return 0x00f60001;
	case FGR_RAAC:	return 0xffff0002;
	case FGR_RSAC:	return 0xffff0002;
	case FGR_RARC:	return 0x00ff0001;
	case FGR_RSRC:	return 0x00ff0001;
	case FGR_TEMP:	return 0xffe00512;
	case FGR_VOLT:	return 0xffe00512;
	case FGR_AS:	return 0x00ff0001;
	case FGR_AC:	return 0xffff0002;
	case FGR_VCHG:	return 0x00ff0001;
	case FGR_IMIN:	return 0x00ff0001;
	case FGR_VAE:	return 0x00ff0001;
	case FGR_IAE:	return 0x00ff0001;
	case FGR_RSP:	return 0x00ff0001;
	case FGR_ADDR:	return 0x00fe0101;
	case FGR_IAVG:
	case FGR_CRNT:
	case FGR_ACR:
	case FGR_ACRL:
	case FGR_SFR:
	case FGR_FULL:
	case FGR_AE:
	case FGR_SE:
	case FGR_PROM:
	case FGR_USER:
	case FGR_CTRL:
	case FGR_AB:
	case FGR_RSG:
	case FGR_RSTC:
	case FGR_FRSG:
			return EMISSING;
	default:	return EARG;
	}
}

/// Byte-swap a 16-bit value
#define SWAP( val ) (((val)<<8) | (((val)>>8)&0xff))

/** The appropriate amount of data for the specified register is read from the
 *  DS2782, then converted to int32 according to the specification in the
 *  datasheet (reserved bits are masked off, the useful data is shifted as
 *  appropriate, signed registers are sign-extended to 32 bits, and unsigned
 *  registers are padded with zeros). Communication with the DS2782 may fail,
 *  in which case a special error value is returned instead of the register
 *  contents. The FGG_ERROR() macro should always be used to check the return
 *  value for error before it is used in any computations.
 *
 *  \param addr
 *	The address of the register to read.
 *
 *  \return
 *	A value which must be checked for error before it can be used.
 *	FGG_ERROR( val ) will evaluate to one of:
 *  \retval ESUCCESS
 *	Communication completed without error. The register value is
 *	accessible in \a val.
 *  \retval EARG
 *	\a addr is not a valid register.
 *  \retval EMISSING
 *	Code to read register \a addr is not implemented yet. Fill in a case
 *	in select_action() to implement it.
 *  \retval x
 *	Where \a x is any error code that can be returned by fg_get_multi().
 */
int32_t fg_get( fg_register_t addr )
{
	register_storage_t v;
	uint32_t action;
	error_t status;
	int32_t val;

	action = select_action( addr );
	if( ACT_FAIL(action) )
		return FGG_ERRBASE + ACT_ERRC( action );

	// now read the value and return it as a 32-bit int
	status = fg_get_multi( addr, &v, ACT_SIZE(action) );
	if( status != ESUCCESS )
		return FGG_ERRBASE + status;

	// correctness of this switch depends on SWAP() preserving signedness
	// mask must be applied before assignment to val, for sign extension
	// of course if any signed field does not occupy the high bits of its
	// register, the masking code will need to be changed to handle that
	switch( ACT_SIZE(action) ) {
	case 0x01:	val = v.u1 & ACT_MASK(action);		break;
	case 0x02:	val = SWAP(v.u2) & ACT_MASK(action);	break;
	case 0x11:	val = v.s1 & ACT_MASK(action);		break;
	case 0x12:	val = SWAP(v.s2) & ACT_MASK(action);	break;
	default:	DBGN( "bad" );				break;
	}

	return val >> ACT_SAMT( action );
}

/** Like fg_get_multi(), but writes a register instead of reading it.
 */
error_t fg_set_multi( uint8_t addr, void *buf, uint8_t size )
{
	error_t status;
	uint8_t *from = (uint8_t*)buf;

	Wire.beginTransmission( fg_addr );
	Wire.write( addr );
	while( size-- > 0 )
		Wire.write( *from++ );

	return wire_status( Wire.endTransmission() );
}

/** Like fg_get(), but writes a register instead of reading it. If \a addr is
 *  FGR_ADDR, the required FGR_SFR write to set SAWE to 1 is automatically
 *  issued.
 *
 *  \warning The static #fg_addr value is NOT updated after FGR_ADDR is
 *  written, so changing FGR_ADDR is only useful within this file.
 *
 *  \param addr
 *	The address of the register to write.
 *  \param val
 *	The value to write, represented as an int32.
 *
 *  \retval ESUCCESS
 *	Communication completed without error.
 *  \retval EARG, EMISSING
 *	See fg_get().
 *  \retval x
 *	Where \a x is any error code that can be returned by fg_set_multi().
 */
error_t fg_set( fg_register_t addr, int32_t val )
{
	register_storage_t v;
	uint32_t action;

	action = select_action( addr );
	if( ACT_FAIL(action) )
		return ACT_ERRC( action );

	// a bit hokey...the idea is to have a single call to fg_set_multi()
	val = (val << ACT_SAMT(action)) & ACT_MASK( action );
	switch( ACT_SIZE(action) ) {
	case 1:	v.u1 = val;		break;
	case 2:	v.u2 = SWAP(val);	break;
	default:DBGN( "bad" );		break;
	}

	if( addr == FGR_ADDR ) {
		// must set SAWE to 1 before the device will allow its slave
		// address to be remapped
		error_t status;
		uint8_t tmp = 3;
		status = fg_set_multi( FGR_SFR, &tmp, sizeof(tmp) );
		if( status != ESUCCESS )
			return status;
	}

	// finally the actual register write can be done
	return fg_set_multi( addr, &v, ACT_SIZE(action) );
}
