/*************************************************************************//**
 * \file   fuelgauge.h
 * \brief  Declarations for communication with the MAXIM DS2782 fuel gauge IC.
 * \author Jon Lindsay (Jonathan.Lindsay@asu.edu)
 * \date   20091025 - initial version
 ****************************************************************************/

#ifndef FUELGAUGE_H
#define FUELGAUGE_H

#include"error.h"

/// See FGG_STATUS().
/** fg_get() returns a value greater than this to indicate error.
 *  FGG_STATUS should be used to determine whether fg_get() was successful,
 *  rather than using FGG_ERRBASE directly.
 */
#define FGG_ERRBASE 0x10000

/// Macro to convert an fg_get() return value into an error_t status code
/** Evaluates to ESUCCESS if the fg_get() call completed without error, EARG
 *  if the requested address is not a valid register, or an appropriate
 *  error_t value if communication with the DS2782 failed.
 */
#define FGG_STATUS(r) ((r)>FGG_ERRBASE? (error_t)((r)-FGG_ERRBASE) : ESUCCESS)

/** \brief Addresses of various DS2782 registers, for use with fg_get() /
 *  fg_set() and variants.
 *
 *  R = read access, W = write access, A = automatically updated by the fuel
 *  gauge while the system is running.
 */
typedef enum fg_register_e {
	FGR_STAT =0x01,	///<Status Register;			1 byte	R/W
	FGR_RAAC =0x02,	///<Remaining Active Absolute Capacity;	 2 bytes R
	FGR_RSAC =0x04,	///<Remaining Standby Absolute Capacity; 2 bytes R
	FGR_RARC =0x06,	///<Remaining Active Relative Capacity;	 1 byte	R
	FGR_RSRC =0x07,	///<Remaining Standby Relative Capacity; 1 byte	R
	FGR_IAVG =0x08,	///<Average Current Register;		2 bytes	R
	FGR_TEMP =0x0a,	///<Temperature Register;		2 bytes	R
	FGR_VOLT =0x0c,	///<Voltage Register;			2 bytes	R
	FGR_CRNT =0x0e,	///<Current Register;			2 bytes	R
	FGR_ACR  =0x10,	///<Accumulated Current Register;	2 bytes	R/W, A
	FGR_ACRL =0x12,	///<Low Accumulated Current Register;	2 bytes	R
	FGR_AS   =0x14,	///<Age Scalar;				1 byte	R/W, A
	FGR_SFR  =0x15,	///<Special Feature Register;		1 byte	R/W
	FGR_FULL =0x16,	///<Full Capacity;			2 bytes	R
	FGR_AE   =0x18,	///<Active Empty;			2 bytes	R
	FGR_SE   =0x1a,	///<Standby Empty;			2 bytes	R
	FGR_PROM =0x1f,	///<EEPROM register;			1 byte	R/W
	FGR_USER =0x20,	///<User EEPROM (Block 0);		24 bytes R/W
	FGR_CTRL =0x60,	///<Control Register;			1 byte	R/W
	FGR_AB   =0x61,	///<Accumulation Bias;			1 byte	R/W
	FGR_AC   =0x62,	///<Aging Capacity;			2 bytes	R/W
	FGR_VCHG =0x64,	///<Charge Voltage;			1 byte	R/W
	FGR_IMIN =0x65,	///<Minimum Charge Current;		1 byte	R/W
	FGR_VAE  =0x66,	///<Active Empty Voltage;		1 byte	R/W
	FGR_IAE  =0x67,	///<Active Empty Current;		1 byte	R/W
	FGR_RSP  =0x69,	///<Sense Resistor Prime;		1 byte	R/W
	FGR_RSG  =0x78,	///<Sense Resistor Gain;		2 bytes	R/W
	FGR_RSTC =0x7a,	///<Sense Resistor Temp. Coeff.;	1 byte	R/W
	FGR_FRSG =0x7b,	///<Factory Gain;			2 bytes	R/W
	FGR_ADDR =0x7e	///<2-Wire Slave Address;		1 byte	R/W
} fg_register_t;
#define FGR_PARM FGR_CTRL	///<Parameter EEPROM (Block 1); 32 bytes R/W

/// Function commands recognized by the DS2782, for use with fg_func()
typedef enum fg_func_e {
	FGF_SAVE_0 = 0x42,	///< Copy from shadow RAM to EEPROM block 0
	FGF_SAVE_1 = 0x44,	///< Copy from shadow RAM to EEPROM block 1
	FGF_LOAD_0 = 0xb2,	///< Copy from EEPROM block 0 to shadow RAM
	FGF_LOAD_1 = 0xb4,	///< Copy from EEPROM block 1 to shadow RAM
	FGF_LOCK_0 = 0x63,	///< *PERMANENTLY* lock EEPROM block 0
	FGF_LOCK_1 = 0x66	///< *PERMANENTLY* lock EEPROM block 1
} fg_func_t;


/// Initialize fuel gauge functions to use \a addr as the DS2782 slave address
error_t fg_init( uint8_t addr );

/// Call a DS2782 builtin function
error_t fg_func( fg_func_t func );
/// Read \a size bytes from DS2782 into \a buf, starting at \a addr
error_t fg_get_multi( uint8_t addr, void *buf, uint8_t size );
/// Read a particular register from DS2782 and return its value as int32
int32_t fg_get( fg_register_t addr );
/// Write \a size bytes to DS2782 from \a buf, starting at \a addr
error_t fg_set_multi( uint8_t addr, void *buf, uint8_t size );
/// Write a particular register to DS2782
error_t fg_set( fg_register_t addr, int32_t val );

#endif
