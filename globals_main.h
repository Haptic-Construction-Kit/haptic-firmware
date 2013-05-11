/*************************************************************************//**
 * \file   globals_main.h
 * \brief  Structure for global items on the Funnel, defined in main.c.
 * \author Jon Lindsay (Jonathan.Lindsay@asu.edu)
 * \date   20090510 - initial version
 ****************************************************************************/

#ifndef GLOBALS_H
#define GLOBALS_H

#include"active_command.h"
#include"parse.h"
#include"menu.h"

/// Current version of the funnel firmware--must be an ASCII decimal number
#define FUNNEL_VER "0"

/// Expected version of the motor modules
#define TINY_VER 0

/** \brief Maximum number of motors the firmware can support.
 *  Careful--increasing this requires a change in the active command format!
 */
#define MAX_MOTORS 16

/// Possible belt operation modes
typedef enum {
	M_LEARN,	///<Learning mode: ASCII commands. See parse_step_t
	M_ACTIVE	///<Active mode: raw byte stream. See active_command_t
} mode_t;

/// Globals used on the Funnel I/O board
typedef struct {
	/** \brief Common buffer used to receive commands over serial, send
	 *  commands over TWI, and send responses over serial.
	 */
	char cmd[ PARSE_MAX_LEN ];

	/// Used to receive active commands over serial and relay over TWI
	active_command_t acmd;

	/// Mapping of motor numbers to TWI addresses, with flag for TWI error
	struct { uint8_t addr:7, err:1; } mtrs[ MAX_MOTORS+1 ];

	/// Current belt mode
	mode_t mode;

	// various global flags
	uint8_t in_menu:1,	///<Set to 1 if user is in the debug menu
		echo:1,		///<If 1, echo all serial input back to user
		fuel_gauge:1;	///<Set to 1 when the fuel gauge IC is present

	/// (Sub)menu currently being displayed to the user
	menu_step_t menustep;
} globals_t;

/// Globals available to all Funnel I/O code
extern globals_t glbl;

#endif
