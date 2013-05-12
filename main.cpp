/*************************************************************************//**
 * \file   main.cpp
 * \brief  Main firmware code for the Funnel I/O.
 * \author Jon Lindsay (Jonathan.Lindsay@asu.edu)
 * \date   20090510 - initial version
 ****************************************************************************/

#include <Wire.h>
#include <EEPROM.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
//#include <avr/eeprom.h>	// arduino apparently breaks this

#include "error.h"
#include "parse.h"
#include "rhythm.h"
#include "magnitude.h"
#include "vibration.h"
#include "wire_err.h"
#include "globals_main.h"
#include "menu.h"
//#include "fuelgauge.h"

#include "debug_main.h"

// because arduino is broken...
// if the headers are wrapped with extern "C" to make it so these don't need
// to be #included, then the funnel simply doesn't run any more
#include "error.c"
#include "parse.c"

/// Time to wait for a status response from a motor, in ms
#define TWI_TIMEOUT 100

/// Time to wait for vibrator modules to stabilize, in ms
#define TINY_WAIT 1000

/// Fuel gauge TWI address
#define FG_TWI_ADDR 0x7f

/// Support motor slave addresses between 0 and this number, exclusive
#define MAX_TWI_ADDR 0x7f

/// Offset of magnitude storage in the EEPROM
#define EE_MAG ((magnitude_t*)0)
/// Offset of rhythm storage in the EEPROM
#define EE_RHY ((rhythm_t*)(EE_MAG + MAX_MAGNITUDE))

/// Funnel globals, defined in globals_main.h
globals_t glbl;

/// Print a string from program space over the serial link
void print_flash( const char* );

/// Read a chunk of data from the EEPROM
// only because arduino is broken and the avr-libc eeprom_*() don't work
static inline void eeprom_read( void* into, void* from, size_t len )
{
	size_t i;
	for( i=0; i<len; ++i )
		*((uint8_t*)into + i) = EEPROM.read( (size_t)from + i );
}

/// Write a chunk of data to the EEPROM
static inline void eeprom_write( void* into, void* from, size_t len )
{
	size_t i;
	for( i=0; i<len; ++i )
		EEPROM.write( (size_t)into+i, *((uint8_t*)from+i) );
}

/// Zero a chunk of the EEPROM
static inline void eeprom_zero( void* start, void* end )
{
	while( start < end ) {
		EEPROM.write( (size_t)start, 0 );
//		eeprom_write_byte( (uint8_t*)start, 0 );
		start = (uint8_t*)start + 1;
	}
}

/// Convert an unsigned byte to two ASCII hex digits plus null terminator
void itoh( char *into, uint8_t val )
{
	into[0] = val >> 4;
	into[0] += ( into[0]<10? '0' : 'A'-10 );
	into[1] = val & 0xf;
	into[1] += ( into[1]<10? '0' : 'A'-10 );
	into[2] = '\0';
}

/// Send the command in the global command buffer to the specified motor
// if motor is specified as -1, send command to all motors via a general call
error_t send_command( int8_t motor )
{
	unsigned long start;
	uint8_t status;

	// make sure the requested motor is present
	if( motor >= MAX_MOTORS ) return ENOMOTOR;

	if( motor < 0 )
		motor = 0;	// general call
	else {
		// find the actual TWI address of the given motor
		motor = glbl.mtrs[ motor ].addr;
		if( !motor ) return ENOMOTOR;
	}

	// send the command over TWI
	// choose which buffer to send based on the current belt mode
	Wire.beginTransmission( motor );
	if( glbl.mode == M_ACTIVE )
		Wire.write( *((uint8_t*)&glbl.acmd.v) );
	else	// learning mode
		Wire.write( glbl.cmd );
	status = Wire.endTransmission();

	// if the TWI transmission failed, return a specific bus error status
	if( status ) {
		if( status >= 4 )
			return EBUS;
		else	return (error_t)(EBUS + status);
	}

	// must not try to request data with a general call...
	// FIXME? anything else to do?
	if( motor == 0 ) return ESUCCESS;

	// without a delay the tiny misses the status request...
	// wait for at most TWI_TIMEOUT milliseconds
	for( start=millis(); millis()-start < TWI_TIMEOUT; )
		if( Wire.requestFrom((uint8_t)motor, (uint8_t)1) )
			return (error_t)Wire.read();

	return EBUS;
}

/// Send a command to each attached motor in turn
// mark any motor that returns an error
// FIXME: do something when an error occurs
error_t send_command_all( void )
{
	uint8_t i, errors = 0;

	for( i=0; glbl.mtrs[i].addr; ++i ) {
		error_t ret = send_command(i);
		glbl.mtrs[i].err = ret != ESUCCESS;
		errors |= glbl.mtrs[i].err;
		DBGC(" ");
		DBGC( ret, DEC );
	}
	DBGCN("");

	if( errors ) return EBUS;

	return ESUCCESS;
}

/// Detect the addresses of all motors present on the TWI bus
uint8_t detect_motors( void )
{
	uint8_t i, j = 0;

	// send a command on every address
	// if the command is ACKed, then a motor is present
	DBG( "dm:" );
	for( i=1; i<MAX_TWI_ADDR && j<MAX_MOTORS; ++i ) {
		wire_err_t ret;

		DBGC( i, HEX );
		Wire.beginTransmission(i);
		Wire.write(0);
		ret = (wire_err_t)Wire.endTransmission();

		if( ret != WE_ANACK ) {
			glbl.mtrs[j].addr = i;
			if( ret != WE_SUCCESS ) {
				DBGC("/");
				glbl.mtrs[j].err = 1;
			}else {
				DBGC("+");
			}
			++j;
		}else {
			DBGC("-");
		}
	}
	DBGCN("");
	for( i=j; i<=MAX_MOTORS; ++i )
		glbl.mtrs[i].addr = 0;

	// print a debug message that shows addresses of all detected motors
#ifdef DEBUG
	DBG( j, DEC ); DBGC( " motors:" );
	for( int i=0; glbl.mtrs[i].addr; ++i ) {
		DBGC( " " );
		DBGC( glbl.mtrs[i].addr, HEX );
	}
	DBGCN("");
#endif

	return j;
}

/// Generate an ASCII representation of a rhythm
error_t rtos( char *into, uint8_t which )
{
	rhythm_t rhy;
	uint8_t i;

	// retrieve the specified rhythm from EEPROM
	eeprom_read( &rhy, EE_RHY+which, sizeof(rhy) );
	if( !rhy.bits || rhy.bits>MAX_RBITS )
		return ENOR;

	strcpy( into, "RHY " );
	into += 4;
	*into++ = itol( which );
	*into++ = ' ';
	for( i=0; i<sizeof(rhy.pattern); ++i, into+=2 )
		itoh( into, rhy.pattern[i] );
	*into++ = ' ';
	utoa( rhy.bits, into, 10 );

	return ESUCCESS;
}

/// Generate an ASCII representation of a magnitude
error_t mtos( char *into, uint8_t which )
{
	magnitude_t mag;

	// retrieve the specified magnitude from EEPROM
	eeprom_read( &mag, EE_MAG+which, sizeof(mag) );
	if( !mag.period ) return ENOM;

	strcpy( into, "MAG " );
	into += 4;
	*into++ = itol( which );
	*into++ = ' ';
	utoa( mag.period, into, 10 );
	into += strlen( into );
	*into++ = ' ';
	utoa( mag.duty, into, 10 );

	return ESUCCESS;
}

/// Load all rhythms and magnitudes from EEPROM and relay them to a motor
// if motor is specified as -1, send to all motors
void teach_motor( uint8_t motor )
{
	uint8_t i;

	DBGN( "rhy" );
	strcpy( glbl.cmd, "LRN " );
	for( i=0; i<MAX_RHYTHM; ++i ) {
		if( rtos(glbl.cmd+4, i) != ESUCCESS )
			continue;
		DBGN( glbl.cmd );
		if( motor == -1 )
			send_command_all();
		else
			send_command( motor );
	}

	DBGN( "mag" );
	for( i=0; i<MAX_MAGNITUDE; ++i ) {
		if( mtos(glbl.cmd+4, i) != ESUCCESS )
			continue;
		DBGN( glbl.cmd );
		if( motor == -1 )
			send_command_all();
		else
			send_command( motor );
	}
}

/// Read a single character from the serial link
/** Busy wait until a character arrives. Convert '\\r' to '\\n'. If \a echo is
 *  non-zero, echo the character back over the serial link, converting '\\r'
 *  to "\r\n" in the echo only.
 */
static inline char read_char( uint8_t echo )
{
	char ch;

	while( !Serial.available() );

	ch = toupper( Serial.read() );
	if( echo ) {
		Serial.print(ch);
		if( ch == '\r' ) Serial.print( '\n' );
	}

	return ch=='\r'? '\n' : ch;
}

/// Print an error_t status value to the serial link, in the form "STS x\n"
static inline void print_status( error_t status )
{
	Serial.print( "STS " );
	Serial.println( status, DEC );
}

/// Read a line of text from serial link into the global ASCII command buffer
void read_line( void )
{
	uint8_t i;
	char ch;

	// erghhh...
	do{
		// read characters until a newline
		i = 0;
		while( (ch=read_char(glbl.echo)) != '\n' )
			if( i < sizeof(glbl.cmd) )
				glbl.cmd[ i++ ] = ch;

		// if the line was too long, return an error
		if( i >= sizeof(glbl.cmd) ) {
			if( glbl.echo )
				print_flash( errstr(ETOOBIG) );
			else
				print_status( ETOOBIG );
		}
	}while( i >= sizeof(glbl.cmd) );

	glbl.cmd[i] = '\0';
}

/// Read two raw bytes from serial link into global activate command buffer
void read_active( void )
{
	uint8_t i = 0;

	while( i < sizeof(active_command_t) )
		if( Serial.available() )
			*((uint8_t*)&glbl.acmd + i++) = Serial.read();
}

/// Same as read_active(), but read 4 ASCII hex digits instead of 2 raw bytes
void read_active_hex( void )
{
	uint8_t *into = (uint8_t*)&glbl.acmd, i = 0;

	while( i++ < sizeof(active_command_t) )
		*into++ = htoi(read_char(1))<<4 | htoi(read_char(1));
}

// command handlers

/// Handle the LRN RHY command. Stores a rhythm in EEPROM and teaches motors.
error_t learn_rhythm( int argc, const char *const *argv )
{
	rhythm_t rhy;
	error_t ret;

	// parse the rhythm and store it in EEPROM
	ret = parse_rhythm( argc, argv, &rhy );
	if( ret != ESUCCESS ) return ret;

	eeprom_write( EE_RHY+(ltoi(0)), &rhy, sizeof(rhy) );

	// relay the learn to all connected motors
	DBG( "relaying rhythm:" );
	send_command_all();

	return ret;
}

/// Handle the LRN MAG command. Stores magnitude in EEPROM and teaches motors.
error_t learn_magnitude( int argc, const char *const *argv )
{
	magnitude_t mag;
	error_t ret;

	// parse the magnitude and store it in EEPROM
	ret = parse_magnitude( argc, argv, &mag );
	if( ret != ESUCCESS ) return ret;

	eeprom_write( EE_MAG+(ltoi(0)), &mag, sizeof(mag) );

	// relay the learn to all connected motors
	DBG( "relaying magnitude:" );
	send_command_all();

	return ret;
}

error_t learn_spatio( int argc, const char *const *argv )
{ return EMISSING; }

error_t learn_address( int argc, const char *const *argv )
{ return EMISSING; }

/// Common work shared by query_rhythm() / query_magnitude()
error_t query_generic( int argc, const char *const *argv,
	uint8_t max, error_t (*func)( char*, uint8_t )
) {
	uint8_t start, finish;

	if( argc > 1 ) return EARG;

	if( argc > 0 ) {
		start = ltoi(0);
		finish = start + 1;
		if( start >= max )
			return EARG;
	}else {
		start = 0;
		finish = max;
	}

	strcpy( glbl.cmd, "RSP " );
	while( start < finish ) {
		if( func(glbl.cmd+4, start) == ESUCCESS )
			Serial.println( glbl.cmd );
		++start;
	}

	return ESUCCESS;
}

/// Handle the QRY RHY command. Prints all stored rhythms to the serial link.
error_t query_rhythm( int argc, const char *const *argv )
{ return query_generic( argc, argv, MAX_RHYTHM, rtos ); }

/// Handle the QRY MAG command. Prints all stored magnitudes to serial link.
error_t query_magnitude( int argc, const char *const *argv )
{ return query_generic( argc, argv, MAX_MAGNITUDE, mtos ); }

error_t query_spatio( int argc, const char *const *argv )
{ return EMISSING; }

/// Handle the QRY MTR command. Prints the number of attached motors.
error_t query_motors( int argc, const char *const *argv )
{
	uint8_t num_motors, old_num;

	if( argc ) return EARG;

	// redetect motors, so that modules can be hot-added or -removed
	// do it here instead of automatically with a periodic TWI poll,
	// because the high-level app will have to change its behavior to
	// accommodate the hardware change
	for( old_num=0; glbl.mtrs[old_num].addr; ++old_num );
	num_motors = detect_motors();
	if( num_motors != old_num )
		teach_motor(-1);

	strcpy( glbl.cmd, "RSP MTR " );
	utoa( num_motors, glbl.cmd+8, 10 );

	Serial.println( glbl.cmd );

	return ESUCCESS;
}

/// Handle the QRY VER command. Prints contents of FUNNEL_VER.
error_t query_version( int argc, const char *const *argv )
{
	if( argc ) return EARG;

	Serial.println( "RSP VER " FUNNEL_VER );

	return ESUCCESS;
}

/// Handle the QRY BAT command. Prints percent battery remaining, in decimal.
// error_t query_battery( int argc, const char *const *argv )
// {
// 	int32_t bat;

// 	if( argc ) return EARG;
// 	if( !glbl.fuel_gauge ) return EMISSING;

// 	bat = fg_get( FGR_RARC );
// 	if( FGG_STATUS(bat) != ESUCCESS )
// 		return FGG_STATUS( bat );

// 	Serial.print( "RSP BAT " );
// 	Serial.println( bat, DEC );

// 	return ESUCCESS;
// }

/// Handle the QRY ALL command. Issues QRY VER,MTR,RHY,MAG,BAT.
error_t query_all( int argc, const char *const *argv )
{
	if( argc ) return EARG;

	query_version( 0, NULL );
	query_motors( 0, NULL );
	query_rhythm( 0, NULL );
	query_magnitude( 0, NULL );
//	query_battery( 0, NULL );

	return ESUCCESS;
}

/// TST command, tries rhythm/magnitude without learning. Not implemented.
error_t test( int argc, const char *const *argv )
{ return EMISSING; }

/// Handle the BGN command. Switches from learning mode to activate mode.
error_t begin( int argc, const char *const *argv )
{
	if( argc ) return EARG;

	glbl.mode = M_ACTIVE;

	return ESUCCESS;
}

/// Handle the ZAP command. Erases all rhythms and magnitudes from EEPROM.
error_t erase_all_learned( int argc, const char *const *argv )
{
	if( argc != 3 ) return EARG;

	eeprom_zero( EE_MAG, EE_RHY+MAX_RHYTHM );

	return ESUCCESS;
}

// parse table definitions
// PROGMEM is supposed to come *after* the variable name, but then doxygen
// can't figure it out

/// Table of recognized learn commands. Hit after a LRN word is parsed.
static PROGMEM const parse_step_t pt_learn[] = {
	{ "RHY", NULL, learn_rhythm },
	{ "MAG", NULL, learn_magnitude },
	{ "SPT", NULL, learn_spatio },
	{ "ADD", NULL, learn_address },
	{ "", NULL, NULL }
};

/// Table of recognized query commands. Hit after a QRY word is parsed.
static PROGMEM const parse_step_t pt_query[] = {
	{ "RHY", NULL, query_rhythm },
	{ "MAG", NULL, query_magnitude },
	{ "SPT", NULL, query_spatio },
	{ "MTR", NULL, query_motors },
	{ "VER", NULL, query_version },
//	{ "BAT", NULL, query_battery },
	{ "ALL", NULL, query_all },
	{ "", NULL, NULL }
};

/// Top-level parse table. Used to recognize the first word of a command.
static PROGMEM const parse_step_t pt_top[] = {
	{ "LRN", pt_learn, NULL },
	{ "QRY", pt_query, NULL },
	{ "TST", NULL, test },
	{ "BGN", NULL, begin },
	{ "ZAP", NULL, erase_all_learned },
	{ "", NULL, NULL }
};

/// Try very hard to make a particular motor vibrate even with flaky hardware
/** Retransmits an activate command if the initial transmission results in
 *  status of unrecognized rhythm/magnitude/spatio-temporal. Usual/expected
 *  reason for such a status is that the power connection was temporarily
 *  lost, due to unreliable cable connectors.
 */
static error_t reliable_activate( void )
{
	// try to send the activate command
	error_t status = send_command( glbl.acmd.motor );
	DBG( "status " );
	DBGCN( (int)status );

	// if the motor doesn't know about the rhythm/magnitude, but we think
	// it should, then the motor probably just lost power temporarily
	//
	// but checking to see whether the rhythm/magnitude is defined
	// requires reading EEPROM, so just assume that it is defined and
	// refresh the motor--this is faster in the expected case (the motor
	// lost power), and the user knows which rhythms/magnitudes are
	// defined and so can easily avoid trying to activate undefined ones
	switch( status ) {
	case ENOR:
	case ENOM:
	case ENOS:
		// resend the learn commands to the motor
		DBG( "refreshing motor " );
		DBGCN( glbl.acmd.motor, DEC );
		glbl.mode = M_LEARN;
		teach_motor( glbl.acmd.motor );
		glbl.mode = M_ACTIVE;

		// and retry the activate command
		return send_command( glbl.acmd.motor );
	default:
		// some "real" failure, not just an unrecognized rhythm/etc.
		// on the motor, so no sense in retrying the command
		return status;
	}
}

/// Handle a command in activate mode
error_t parse_active( void )
{
	switch( glbl.acmd.mode ) {
	case ACM_VIB:	return reliable_activate();
	case ACM_SPT:	return EMISSING;
	case ACM_GCL:	return send_command(-1);
	case ACM_LRN:	glbl.mode = M_LEARN;
			return ESUCCESS;
	default:	return EBADCMD;
	}
}

/// Handle a command in learning mode
error_t handle_learn( void )
{
	char cpy[ sizeof(glbl.cmd) ];

	// make a copy of the command, because parse() modifies its argument
	// in place and the learn handlers expect an unmodified glbl.cmd
	strcpy( cpy, glbl.cmd );

	return parse( pt_top, cpy );
}

// menu tables
// again, PROGMEM really belongs after the variable name

/// Top-level menu options. Printed when the user first enters the debug menu.
static PROGMEM const char menu_str_top[] =
	"0. Exit menu\n\r"
	"1. Query commands\n\r"
	"2. Learn commands\n\r"
	"3. Activate a motor\n\r"
	"4. Raw command entry\n\r"
;

/// Query menu options.
static PROGMEM const char menu_str_qry[] =
	"0. Return to main menu\n\r"
	"1. Query belt version\n\r"
	"2. Query number of motors present\n\r"
	"3. Query defined rhythms\n\r"
	"4. Query defined magnitudes\n\r"
//	"5. Query remaining battery\n\r"
	"5. Query all belt information\n\r"
;

/// Learn menu options.
static PROGMEM const char menu_str_lrn[] =
	"0. Return to main menu\n\r"
	"1. Learn rhythm\n\r"
	"2. Learn magnitude\n\r"
	"3. Forget all rhythms and magnitudes\n\r"
;

/// Instructions for the learn rhythm menu.
static PROGMEM const char menu_str_lrn_rhy[] =
	"Enter rhythm ID, pattern, and number of bits, then press ENTER.\n\r"
	"Press ENTER on a blank line when finished.\n\r"
	"\n\r"
	"The rhythm pattern consists of 16 hexadecimal digits. Each bit\n\r"
	"of the pattern represents 50 ms of the rhythm. If a bit is set,\n\r"
	"the motor will vibrate for the corresponding 50 ms during rhythm\n\r"
	"playback; if a bit is cleared, those 50 ms will elapse without\n\r"
	"any vibration.\n\r"
	"\n\r"
	"The number of bits argument specifies how many of the 64 bits\n\r"
	"specified by the pattern are actually used in the rhythm.\n\r"
	"\n\r"
	"Example: A F0C1F00000000000 17<ENTER> defines rhythm A to be\n\r"
	"17 * 50 ms = 850 ms long: 200 ms ON, 200 ms OFF, 100 ms ON, 250\n\r"
	"ms OFF, and finally ON for the last 100 ms.\n\r"
;

/// Instructions for the learn magnitude menu.
static PROGMEM const char menu_str_lrn_mag[] =
	"Enter magnitude ID, period, and pulse width, in microseconds,\n\r"
	"then press ENTER. Press ENTER on a blank line when finished.\n\r"
	"\n\r"
	"To specify full ON (digital 1), enter the same number for both\n\r"
	"period and pulse width.\n\r"
	"\n\r"
	"Example: C 2000 500<ENTER> defines magnitude C to have a 2 ms\n\r"
	"period with 25% duty cycle.\n\r"
;

/// Confirmation prompt for the erase rhythms/magnitudes menu option.
static PROGMEM const char menu_str_forget[] =
	"All defined rhythms and magnitudes will be erased from EEPROM.\n\r"
	"Continue?\n\r"
	"0. No\n\r"
	"1. Yes\n\r"
;

/// Instructions for the activate motor menu.
static PROGMEM const char menu_str_act[] =
	"Enter motor, rhythm, magnitude, and duration, then press ENTER.\n\r"
	"Press ENTER on a blank line when finished.\n\r"
	"\n\r"
	"Example: CED6<ENTER> will activate the third motor for 6 cycles\n\r"
	"of rhythm E at magnitude D.\n\r"
;

// menu handlers

/// Issue the QRY VER command from the debug menu
error_t menu_qry_ver( void ) { return query_version(0, NULL); }
/// Issue the QRY MTR command from the debug menu
error_t menu_qry_mtr( void ) { return query_motors(0, NULL); }
/// Issue the QRY RHY command from the debug menu
error_t menu_qry_rhy( void ) { return query_rhythm(0, NULL); }
/// Issue the QRY MAG command from the debug menu
error_t menu_qry_mag( void ) { return query_magnitude(0, NULL); }
/// Issue the QRY BAT command from the debug menu
//error_t menu_qry_bat( void ) { return query_battery(0, NULL); }
/// Issue the QRY ALL command from the debug menu
error_t menu_qry_all( void ) { return query_all(0, NULL); }

/// Common work shared by menu_lrn_rhy() / menu_lrn_mag()
void menu_lrn_generic( const char *prepend )
{
	while(1) {
		// read the rhythm/magnitude specification from the user
		Serial.print( "Specification: " );
		read_line();
		if( glbl.cmd[0] == '\0' ) break;

		// prepend the LRN RHY/MAG
		memmove( glbl.cmd+8, glbl.cmd, sizeof(glbl.cmd)-8 );
		glbl.cmd[ sizeof(glbl.cmd) - 1 ] = '\0';
		memcpy( glbl.cmd, "LRN ", 4 );
		memcpy( glbl.cmd+4, prepend, 4 );

		// handle the command...should always be in learning mode here
		// so not necessary to change glbl.mode
		print_flash( errstr(handle_learn()) );
	}
}

/// Issue the LRN RHY command from the debug menu
error_t menu_lrn_rhy( void )
{
	print_flash( menu_str_lrn_rhy );
	menu_lrn_generic( "RHY " );
	return ESUCCESS;
}
/// Issue the LRN MAG command from the debug menu
error_t menu_lrn_mag( void )
{
	print_flash( menu_str_lrn_mag );
	menu_lrn_generic( "MAG " );
	return ESUCCESS;
}
/// Issue the ZAP command from the debug menu
error_t menu_lrn_forget( void ) { return erase_all_learned(3, NULL); }

#define ARG( _dst_, _base_, _max_ ) \
	val = glbl.cmd[ i++ ] - _base_; \
	if( val >= _max_ ) { \
		Serial.println( "Invalid command" ); \
		continue; \
	} \
	_dst_ = val;
/// Activate a motor from the debug menu
error_t menu_act( void )
{
	print_flash( menu_str_act );

	while(1) {
		uint8_t val, i = 0;	// used by the ARG macro
		mode_t save;

		// read the command from the user
		Serial.print( "Command: " );
		read_line();
		if( glbl.cmd[0] == '\0' ) break;

		// convert it into a native activate command
		ARG( glbl.acmd.motor, 'A', MAX_MOTORS );
		ARG( glbl.acmd.v.rhythm, 'A', MAX_RHYTHM );
		ARG( glbl.acmd.v.magnitude, 'A', MAX_MAGNITUDE );
		ARG( glbl.acmd.v.duration, '0', MAX_DURATION+1 );
		glbl.acmd.mode = 0;

		// send the command
		save = glbl.mode;
		glbl.mode = M_ACTIVE;
		print_flash( errstr(reliable_activate()) );
		glbl.mode = save;
	}

	return ESUCCESS;
}
#undef ARG
/** \brief Enter raw command mode. Exits the menu, turns on serial echo, and
 *  interprets activate mode commands as ASCII hex quads rather than 2 raw
 *  bytes. See read_active_hex().
 */
error_t menu_raw( void ) { glbl.echo = 1; glbl.in_menu = 0; return ESUCCESS; }
/** \brief Exits the debug menu to normal command mode (echo off, activate
 *  commands read as 2 raw bytes). See read_active().
 */
error_t menu_exit( void ) { glbl.in_menu = glbl.echo = 0; return ESUCCESS; }

// "forward declarations" of menu steps not yet defined
extern PROGMEM const menu_step_t menu_choices_top[];
extern PROGMEM const menu_step_t menu_choices_lrn[];

/// Special value to mark the end of a set of menu choices
error_t menu_end( void ) { return EMAX; }

/// Query menu function table. Specifies a handler for each query menu choice.
static PROGMEM const menu_step_t menu_choices_qry[] = {
	{ menu_str_top, menu_choices_top, NULL },
	{ NULL, NULL, menu_qry_ver },
	{ NULL, NULL, menu_qry_mtr },
	{ NULL, NULL, menu_qry_rhy },
	{ NULL, NULL, menu_qry_mag },
//	{ NULL, NULL, menu_qry_bat },
	{ NULL, NULL, menu_qry_all },
	{ NULL, NULL, menu_end }
};

/// Forget menu function table. Handles the erase confirmation prompt.
static PROGMEM const menu_step_t menu_choices_forget[] = {
	{ menu_str_lrn, menu_choices_lrn, NULL },
	{ menu_str_lrn, menu_choices_lrn, menu_lrn_forget },
	{ NULL, NULL, menu_end }
};

/// Learn menu function table. Specifies a handler for each learn menu choice.
PROGMEM const menu_step_t menu_choices_lrn[] = {
	{ menu_str_top, menu_choices_top, NULL },
	{ NULL, NULL, menu_lrn_rhy },
	{ NULL, NULL, menu_lrn_mag },
	{ menu_str_forget, menu_choices_forget, NULL },
	{ NULL, NULL, menu_end }
};

/// Top-level menu table. Specifies handler/submenu for each menu choice.
PROGMEM const menu_step_t menu_choices_top[] = {
	{ NULL, NULL, menu_exit },
	{ menu_str_qry, menu_choices_qry, NULL },
	{ menu_str_lrn, menu_choices_lrn, NULL },
	{ NULL, NULL, menu_act },
	{ NULL, NULL, menu_raw },
	{ NULL, NULL, menu_end }
};

/// Starting point for handle_menu()
static PROGMEM const menu_step_t menu_top[] = {
	{ menu_str_top, menu_choices_top, NULL },
	{ NULL, NULL, menu_end }
};

/// Display the debug menu and handle all menu navigation/function calls.
/** The idea here is to provide a generic set of tables that handle_menu() can
 *  traverse. Menu options that should be displayed to the user are specified
 *  in menu_str_*, while the actual callback functions for each menu choice
 *  are specified in menu_choices_*.
 *
 *  Each entry in a menu_choices_* array counts as one menu option, and can
 *  specify a callback function and/or a submenu table (another
 *  menu_choices_*).
 *
 *  handle_menu() assigns numbers to each entry it finds in a menu_choices_*,
 *  in the order they are defined. It then reads a number from the serial
 *  link, indexes the menu_choices_* table by that number, calls the callback
 *  function for that entry if specified, and moves to the submenu, if
 *  specified.
 */
void handle_menu( void )
{
	menu_step_t step;
	int8_t max, sel;
	error_t status;

	glbl.in_menu = 1;
	glbl.echo = 1;

	while( glbl.in_menu ) {
		const menu_step_t *choice = glbl.menustep.choices;

		// display the current menu
		Serial.println();
		print_flash( glbl.menustep.menu );

		// figure out the number of choices
		// FIXME: (int) should be (uint16_t)
		max = 0;
		while( pgm_read_word(&choice[max].func) != (int)menu_end )
			++max;

		// wait for a valid selection
		Serial.print( "Choice: " );
		for( sel=-1; sel<0 || sel>=max; sel=read_char(0)-'0' );
		Serial.println( sel, DEC );

		// load the selected menu step into RAM
		memcpy_P( &step, choice+sel, sizeof(step) );

		// call the handler function if defined
		// do this even if a submenu is specified, so that e.g. a
		// confirmation menu can specify what menu to return to
		status = ESUCCESS;
		if( step.func != NULL ) {
			Serial.println();
			status = step.func();
		}

		// if a submenu is defined in the tables, move into it
		if( step.choices != NULL )
			glbl.menustep = step;
		else if( step.func == NULL ) {
			Serial.println();
			status = EMISSING;
		}

		if( status != ESUCCESS )
			print_flash( errstr(status) );
	}

	// empty the command/response buffer so that the main loop doesn't
	// try to execute whatever happens to be in there when we return
	glbl.cmd[0] = '\0';
}

/// Top-level firmware initialization function
void setup( void )
{
        //on led
        pinMode(13,OUTPUT);
        digitalWrite(13,HIGH);
        
	unsigned long start;

	Wire.begin();
	Serial.begin( 9600 );

	memset( &glbl, 0, sizeof(glbl) );

#ifdef DEBUG
	for( start=millis(); millis()-start < 3000; );
#endif

	// wait for vibrator microprocessor to stabilize for detection function
	DBGN( "stabilize" );
	for( start=millis(); millis()-start < TINY_WAIT; )
		;

	// set up the fuel gauge before probing for motors, so that it can be
	// moved off of the factory address and out of the motor address space
//	glbl.fuel_gauge = fg_init( 0x7f ) == ESUCCESS;
//	DBGN( fg_get(FGR_TEMP), HEX );

	detect_motors();	// determine which motors are present on bus
	teach_motor(-1);	// relay rhythms/magnitudes to attached motors

	// initialize the menu to the top level
	memcpy_P( &glbl.menustep, menu_top, sizeof(glbl.menustep) );

//	uint16_t cap;
//	ret = fg_get_multi( FGR_TEMP, &cap, sizeof(cap) );
//	DBG( ret, DEC ); DBGC(" "); DBGCN( cap, DEC );

	DBGN( "...done" );
}

/// Main firmware execution loop
void loop( void )
{
	error_t status;

	if( glbl.mode == M_ACTIVE ) {
		if( glbl.echo ) {
			// raw command mode; read the command as ASCII hex
			// and print human-readable status response
			read_active_hex();
			Serial.write( ' ' );
			print_flash( errstr(parse_active()) );
		}else {
			// normal mode; read and reply machine-format bytes
			read_active();
			Serial.write( parse_active() );
		}
	}else {
		uint8_t count = 0;

		do{
			read_line();
			if( count < 2 )
				++count;
			else {
				handle_menu();
				count = 0;
			}
		}while( glbl.cmd[0] == '\0' );
		DBGN( glbl.cmd );

		// handle the command and return status
		status = handle_learn();

		if( glbl.echo ) {
			// raw command mode; print a pretty status message
			print_flash( errstr(status) );
		}else {
			// normal mode; return parsable status
			print_status( status );
		}
	}
}

// arduino is a joke.
unsigned long mswrap() { return millis(); }

/*Author: Kristopher Blair (Haptic-Research-Group)
 *Date: April 22nd, 2009
 *
 *Description: Menu display for Arduino controller, for use with
 *haptic belt research project, Arizona State University.
 *
 */
void print_flash(const char *str)
{ 
	char c;
	if(!str) return;
	while((c = pgm_read_byte(str++))) Serial.write(c);
	Serial.println();
}
