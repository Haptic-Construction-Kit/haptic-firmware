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
 *	ttmmmmmmRRRMMddd
 *  where t = type, m = motor, R = rhythm, M = magnitude, d = duration.
 */
typedef struct {
	uint8_t motor:6,	///<Motor number, 0-63
		mode:2;		///<Command type, see acmd_mode_t
	vibration_t v;		///<Rhythm, magnitude, and duration
} active_command_t;

/// Values for the mode (command type) field of an active mode command
typedef enum {
	ACM_VIB,	///<Activate a motor 00
	ACM_SPT,	///<Play back a spatio-temporal pattern 01
	ACM_GCL,	///<Send a command to all motors (TWI general call) 10
	ACM_LRN		///<Return to learning mode 11
} acmd_mode_t;

#endif
