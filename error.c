/*************************************************************************//**
 * \file   error.c
 * \brief  Strings and functions for human-readable error messages.
 * \details This file is intended to be built in both the firmware and DLL
 *         sides of the code, to provide a common error interface.
 * \author Jon Lindsay (Jonathan.Lindsay@asu.edu)
 * \date   20090504 - initial version
 ****************************************************************************/

#include"error.h"

// this should check for something only defined on avr, but since it's not
// clear what that define would be, this is easy...                       
#ifndef PROGMEM                                                           
	// not compiling for AVR
	// do not attempt to store the strings in program space, and define a
	// dummy pgm_read_word() macro for errstr()
#	define STR const char
#	define pgm_read_word( _addr_ ) *(_addr_)                             
#else
	// building as part of the firmware
	// strings must be stored in program space; pgm_read_word() is
	// provided by the avr libraries to access program space

	// avr docs say preferred PROGMEM location is *after* the variable
	// name, but then doxygen can't figure it out
#	define STR PROGMEM const char
#endif                                                                    

static STR esuccess[] = "Success";
static STR ebadcmd[] = "Command not recognized";
static STR etoobig[] = "Command too long";
static STR earg[] = "Invalid argument";
static STR enor[] = "Requested rhythm not defined";
static STR enom[] = "Requested magnitude not defined";
static STR enos[] = "Requested spatio-temporal pattern not defined";
static STR enomotor[] = "Requested motor not present";
static STR einvr[] = "Invalid rhythm definition";
static STR einvm[] = "Invalid magnitude definition";
static STR einvs[] = "Invalid spatio-temporal pattern definition";
static STR ebadvc[] = "Vibrator command not recognized";
static STR ebus[] = "Bus communication failed";
static STR ebusof[] = "Bus transmit overflow";
static STR ebusan[] = "Bus address not acknowledged";
static STR ebusdn[] = "Bus data not acknowledged";
static STR emissing[] = "Command not implemented";
static STR emax[] = "Unknown error";

/// Table of status strings for fast lookups
/** There must be exactly one entry in this table for each error code defined
 *  in error_t. The entries must appear in the same order as the error_t
 *  definitions.
 *
 *  Status strings are individually named above before being added to the
 *  table so that the actual string contents are stored in program space
 *  (flash) when this file is compiled for AVR. If the strings were supplied
 *  directly in the table, they would be stored in memory and completely
 *  exhaust the available RAM.
 */
static STR *const error_names[] = {
	esuccess,
	ebadcmd,
	etoobig,
	earg,
	enor,
	enom,
	enos,
	enomotor,
	einvr,
	einvm,
	einvs,
	ebadvc,
	ebus,
	ebusof,
	ebusan,
	ebusdn,
	emissing,
	emax
};
#undef STR

/** The status strings will be stored in program space when this function is
 *  compiled into the firmware. The print_flash() function should be used to
 *  send such a string over the serial link.
 *
 *  When compiled into the DLL, the status strings are stored in RAM and can
 *  be accessed as usual.
 *
 *  In all cases, the returned strings are statically allocated and must not
 *  be modified.
 *
 *  \param num The status code to return a string for.
 *  \return A pointer to a statically-allocated status string.
 */
const char* errstr( error_t num )
{
	if( num<0 || num>EMAX )
		return (const char*)pgm_read_word( error_names + EMAX );

	return (const char*)pgm_read_word( error_names + num );
}
