/*************************************************************************//**
 * \file   learn_tiny.c
 * \brief  Command handlers for learning mode in the vibration modules.
 * \author Jon Lindsay (Jonathan.Lindsay@asu.edu)
 * \date   20090426 - initial version
 ****************************************************************************/

#include<stdlib.h>
#include<string.h>

#include"globals.h"
#include"rhythm.h"
#include"magnitude.h"
#include"learn_tiny.h"

//#include"debug.h"

// command handlers

/// LRN RHY \<ID> \<PATTERN> \<BITS>; see parse_rhythm()
static error_t learn_rhythm( int argc, const char *const *argv )
{
	return parse_rhythm( argc, argv, glbl.rhythms+ltoi(0) );
}

/// LRN MAG \<ID> \<PERIOD> \<DUTY>; see parse_magnitude()
static error_t learn_magnitude( int argc, const char *const *argv )
{
	return parse_magnitude( argc, argv, glbl.magnitudes+ltoi(0) );
}

/// Not implemented.
static error_t learn_address( int argc, const char *const *argv )
{
	return EMISSING;
}

/// Not implemented.
static error_t query_version( int argc, const char *const *argv )
{
	return EMISSING;
}

/// Not implemented.
static error_t query_address( int argc, const char *const *argv )
{
	return EMISSING;
}

/// Not implemented.
static error_t test( int argc, const char *const *argv )
{
	return EMISSING;
}

// parse table definitions
// PROGMEM before variable name as not recommended, to make doxygen happy

/// Parse table for learn commands. See parse().
static PROGMEM const parse_step_t pt_learn[] = {
	{ "RHY", NULL, learn_rhythm },
	{ "MAG", NULL, learn_magnitude },
	{ "ADD", NULL, learn_address },
	{ "", NULL, NULL }
};

/// Parse table for query commands. See parse().
static PROGMEM const parse_step_t pt_query[] = {
	{ "VER", NULL, query_version },
	{ "ADD", NULL, query_address },
	{ "", NULL, NULL }
};

/// Top-level parse table. See parse().
static PROGMEM const parse_step_t pt_top[] = {
	{ "LRN", pt_learn, NULL },
	{ "QRY", pt_query, NULL },
	{ "TST", NULL, test },
	{ "", NULL, NULL }
};

error_t handle_learn( char *cmd )
{
	return parse( pt_top, cmd );
}
