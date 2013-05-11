/*************************************************************************//**
 * \file   vibration.h
 * \brief  Definition of the Funnel-to-ATtiny activate command. Also used as
 *         part of the serial activate command to the Funnel I/O.
 * \author Jon Lindsay (Jonathan.Lindsay@asu.edu)
 * \date   20090430 - initial version
 ****************************************************************************/

#ifndef VIBRATION_H
#define VIBRATION_H

#define MAX_DURATION 7	///<Max cycle count for rhythm playback

/// Vibration definition
// FIXME: The Wrong Thing (TM)
// (but it's so easy!)
typedef struct {
	uint8_t duration:3,	///<\# of rhythm cycles to play back; 7 = inf
		magnitude:2,	///<Magnitude to play rhythm at
		rhythm:3;	///<Rhythm to be activated
} vibration_t;

#endif
