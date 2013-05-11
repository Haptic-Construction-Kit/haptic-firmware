/*************************************************************************//**
 * \file   wire_err.h
 * \brief  Error codes for the arduino Wire.endTrasmission() function.
 * \author Jon Lindsay (Jonathan.Lindsay@asu.edu)
 * \date   20090425 - initial version
 ****************************************************************************/

#ifndef WIRE_ERR_H
#define WIRE_ERR_H

/** \brief Possible return codes from Wire.endTransmission(), defined because
 *  the Arduino Wire library failed to provide any enum/defines.
 */
typedef enum {
	WE_SUCCESS,	///<Transmission succeeded
	WE_OVERFLOW,	///<Arduino send buffer overflowed
	WE_ANACK,	///<TWI slave address not acknowledged
	WE_DNACK,	///<TWI data not acknowledged
	WE_ERROR	///<Generic error
} wire_err_t;

#endif
