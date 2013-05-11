/*************************************************************************//**
 * \file   debug.h
 * \brief  Function declarations to aid with debugging.
 * \author Jon Lindsay (Jonathan.Lindsay@asu.edu)
 * \date   20090421 - initial version
 ****************************************************************************/

#ifndef DEBUG_H
#define DEBUG_H

/// Busy-wait with millisecond granularity. Assumes CPU is clocked at 1 MHz.
void delay( uint16_t ms );

/// Dump a byte onto the motor
void dumpbyte( uint8_t byte );

#endif
