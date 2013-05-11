/*************************************************************************//**
 * \file   active_command.h
 * \brief  Active mode command definition.
 * \author Jon Lindsay (Jonathan.Lindsay@asu.edu)
 * \date   20090510 - initial version
 ****************************************************************************/

#ifndef ACTIVE_COMMAND_H
#define ACTIVE_COMMAND_H

#include "vibration.h"

/// Active mode command
/** Commands are 16 bits each, big endian. Format is
 *	ttttmmmmRRRMMddd
 *  where t = type, m = motor, R = rhythm, M = magnitude, d = duration.
 */
typedef struct {
	uint8_t motor:4,	///<Motor number, 0-15
		mode:4;		///<Command type, see acmd_mode_t
	vibration_t v;		///<Rhythm, magnitude, and duration
} active_command_t;

/// Values for the mode (command type) field of an active mode command
typedef enum {
	ACM_VIB,	///<Activate a motor
	ACM_SPT,	///<Play back a spatio-temporal pattern
	ACM_GCL,	///<Send a command to all motors (TWI general call)
	ACM_LRN		///<Return to learning mode
} acmd_mode_t;

#endif
