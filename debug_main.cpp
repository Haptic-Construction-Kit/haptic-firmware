/*************************************************************************//**
 * \file   debug_main.cpp
 * \brief  Debug message prefix; saves a bit of code size.
 * \author Jon Lindsay (Jonathan.Lindsay@asu.edu)
 * \date   20090510 - initial version
 ****************************************************************************/

#include "debug_main.h"

void dbg_prefix( void ) { Serial.print("DBG "); }
