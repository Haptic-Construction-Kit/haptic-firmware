/*************************************************************************//**
 * \file   parse.h
 * \brief  Type definitions for the parser in parse.c.
 * \author Jon Lindsay (Jonathan.Lindsay@asu.edu)
 * \date   20090429 - initial version
 ****************************************************************************/

#ifndef PARSE_H
#define PARSE_H

/* doing this the correct way with extern "C" makes the Funnel not run
#ifdef __cplusplus
extern "C" {
#endif
*/

#include<avr/pgmspace.h>

#include"rhythm.h"
#include"magnitude.h"
#include"error.h"

#define PARSE_MAX_WORDS 10	///<Maximum number of words in each command
#define PARSE_MAX_LEN 32	///<Maximum length of a single command
#define OUT_MAX_LEN 1		///<Maximum length of outbound buffer

/// Convert an ASCII ID letter from the given argument number to an index
#define ltoi( _argnum_ ) ((uint8_t)(*argv[_argnum_] - 'A'))

/// Convert an index to an ASCII ID letter
#define itol( _id_ ) ('A' + _id_)

/// Function pointer type for individual command handlers
typedef error_t (*parse_func_t)( int argc, const char *const *argv );

/** \brief Parse table constituent type. See parse() for details and
 *  learn_tiny.c for examples.
 */
typedef struct parse_step_s {
	/// A word that can occur as part of a valid command
	const char str[4];
	/// Array of words that can occur next if \a str was matched
	const struct parse_step_s *next;
	/// Function to call when \a str is matched and \a next is NULL
	parse_func_t func;
} parse_step_t;

/// Convert an ASCII hex digit into an integer (-1 if not a hex digit)
int8_t htoi( char digit );

/// Convert a rhythm specification into native format at the given location
error_t parse_rhythm( int argc, const char *const *argv, rhythm_t *into );
/// Convert a magnitude specification into native format at the given location
error_t parse_magnitude( int argc, const char *const *argv, magnitude_t *into );

/// Main parser for learning mode commands; modifies \a line in place
error_t parse( const parse_step_t *table, char *line );

/*
#ifdef __cplusplus
} // extern "C"
#endif
*/

#endif
