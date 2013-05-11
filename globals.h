/*************************************************************************//**
 * \file   globals.h
 * \brief  Structure for global items on the ATtiny48, defined in tiny.c.
 * \author Jon Lindsay (Jonathan.Lindsay@asu.edu)
 * \date   20090418 - initial version
 ****************************************************************************/

#ifndef GLOBALS_H
#define GLOBALS_H

#include"rhythm.h"
#include"magnitude.h"

/// Globals used on the ATtiny (motor module firmware)
typedef struct {
	/// Table of all known rhythms
	rhythm_t rhythms[ MAX_RHYTHM ];
	/// Table of all known magnitudes
	magnitude_t magnitudes[ MAX_MAGNITUDE ];
	// above tables are written by learn_rhythm() and learn_magnitude() in
	// learn_tiny.c, and read by rhythm_step() in tiny.c

	// indices for tracking the execution of the active rhythm
	// may be changed mid-execution when a new command arrives on TWI
	int ar;	///<Index into rhythms table of the currently active rhythm
	int ab;	///<Active bit position in rhythms[ar].pattern
	int ac;	///<Remaining number of cycles in active rhythm
} globals_t;

extern globals_t glbl;

#endif
