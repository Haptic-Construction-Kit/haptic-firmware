/*************************************************************************//**
 * \file   error.h
 * \brief  Type definitions and function declarations for error handling.
 * \author Jon Lindsay (Jonathan.Lindsay@asu.edu)
 * \date   20090501 - initial version
 ****************************************************************************/

#ifndef ERROR_H
#define ERROR_H

/* doing this the correct way with extern "C" makes the Funnel not run
#ifdef __cplusplus
extern "C" {
#endif
*/

/// Status codes returned by firmware functions that can fail
/** The information in parentheses indicates the belt modes and communication
 *  paths in which a particular error may occur (not an exhaustive list).
 *  L = learning mode, O = operational (vibrate) mode, P = the PC that
 *  controls the belt via the serial link, M = the main belt controller
 *  (Funnel), V = vibration module controller (ATtiny).
 *
 *  There must be exactly one error code defined here for each string listed
 *  in the error_names table in error.c, and the strings and enum values must
 *  appear in the same order.
 */
typedef enum {
	// symbol	error type (belt mode, command source->destination)
	ESUCCESS,	///< Operation succeeded (no error)
	EBADCMD,	///< Command not recognized		(L/O, P->M)
	ETOOBIG,	///< Command too long			(L, M->V)
	EARG,		///< Invalid argument			(L, P->M/M->V)
	ENOR,		///< Requested rhythm not defined	(O, P->M/M->V)
	ENOM,		///< Requested magnitude not defined	(O, P->M/M->V)
	ENOS,		///< Spatio-temporal pattern not defined (O, P->M/M->V)
	ENOMOTOR,	///< Requested motor not present on belt (O, P->M)
	EINVR,		///< Invalid rhythm definition		(L, P->M/M->V)
	EINVM,		///< Invalid magnitude definition	(L, P->M/M->V)
	EINVS,		///< Invalid spatio-temporal definition	(L, P->M)
	EBADVC,		///< Vibrator command not recognized	(L/O, M->V)
	EBUS,		///< TWI communication failed		(L/O, M->V)
	EBUSOF,		///< TWI transmit overflow		(L/O, M->V)
	EBUSAN,		///< TWI address not acknowledged	(L/O, M->V)
	EBUSDN,		///< TWI data not acknowledged		(L/O, M->V)
	EMISSING,	///< Command not implemented yet	(L, P->M/M->V)
	EMAX		///< Invalid/unknown error number
} error_t;

/// Return a human-readable status string for the given status code
const char* errstr( error_t num );

/*
#ifdef __cplusplus
} // extern "C"
#endif
*/

#endif
