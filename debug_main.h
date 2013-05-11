/*************************************************************************//**
 * \file   debug_main.h
 * \brief  Debug message defines for the main Funnel I/O code.
 * \author Jon Lindsay (Jonathan.Lindsay@asu.edu)
 * \date   20090510 - initial version
 ****************************************************************************/

#ifndef DEBUG_MAIN_H
#define DEBUG_MAIN_H

#include <HardwareSerial.h>

/// If DEBUG is defined, compile in detailed runtime status messages
//#define DEBUG
#ifdef DEBUG
	/// Print a message without the "DBG" prefix
#	define DBGC( ... ) Serial.print( __VA_ARGS__ )
	/// Print a message without the "DBG" prefix, followed by newline
#	define DBGCN( ... ) Serial.println( __VA_ARGS__ )
	/// Print a message with a "DBG" prefix
#	define DBG( ... ) \
		do{ dbg_prefix(); Serial.print(__VA_ARGS__); }while(0)
//		do{ Serial.print("DBG "); Serial.print(__VA_ARGS__); }while(0)
	/// Print a message with a "DBG" prefix, followed by newline
#	define DBGN( ... ) \
		do{ dbg_prefix(); Serial.println(__VA_ARGS__); }while(0)
//		do{Serial.print("DBG ");Serial.println(__VA_ARGS__);}while(0)
#else
#	define DBG( ... )
#	define DBGN( ... )
#	define DBGC( ... )
#	define DBGCN( ... )
#endif

// doing this saves apparently many bytes of code size per DBG
/// Print "DBG " to the serial link. Saves code size for DBG calls.
void dbg_prefix( void );

#endif
