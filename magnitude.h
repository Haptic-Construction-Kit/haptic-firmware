/*************************************************************************//**
 * \file   magnitude.h
 * \brief  The magnitude structure.
 * \author Jon Lindsay (Jonathan.Lindsay@asu.edu)
 * \date   20090413 - initial version
 ****************************************************************************/

#ifndef MAGNITUDE_H
#define MAGNITUDE_H

/// Maximum number of magnitudes that can be learned by the belt
#define MAX_MAGNITUDE 4

/// Magnitude definition
typedef struct {
	uint16_t period;	///<Period, in microseconds
	uint16_t duty;		///<Duty cycle, in us; must be < period
} magnitude_t;

#endif
