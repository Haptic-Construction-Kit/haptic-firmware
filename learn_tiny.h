/*************************************************************************//**
 * \file   learn_tiny.h
 * \brief  Declaration for the learn parser in the tiny.
 * \author Jon Lindsay (Jonathan.Lindsay@asu.edu)
 * \date   20090423 - initial version
 ****************************************************************************/

#ifndef LEARN_TINY_H
#define LEARN_TINY_H

#include"parse.h"

/// Handle a learning command on a motor module. Modifies \a cmd in place.
error_t handle_learn( char *cmd );

#endif
