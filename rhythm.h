/*************************************************************************//**
 * \file   rhythm.h
 * \brief  The rhythm structure.
 * \author Jon Lindsay (Jonathan.Lindsay@asu.edu)
 * \date   20090413 - initial version
 ****************************************************************************/

#ifndef RHYTHM_H
#define RHYTHM_H

#include<inttypes.h>

#define MAX_RHYTHM 8	///<Maximum number of rhythms that can be learned
#define MAX_RBITS 64	///<Number of bits per pattern

/// Rhythm definition
typedef struct {
	uint8_t pattern[8];	///<Each bit represents 50ms; MSB first
	uint8_t bits;		///<Number of pattern bits actually used
} rhythm_t;

#endif
