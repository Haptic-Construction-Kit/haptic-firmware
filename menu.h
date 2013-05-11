/*************************************************************************//**
 * \file   menu.h
 * \brief  Type definitions for the menu parser in menu.c.
 * \author Jon Lindsay (Jonathan.Lindsay@asu.edu)
 * \date   20090510 - initial version
 ****************************************************************************/

#ifndef MENU_H
#define MENU_H

/// Menu handler callback function type
typedef error_t (*menu_func_t)( void );

/// Menu table constituent type. See handle_menu() for details.
typedef struct menu_step_s {
	/// Options to display to the user when this step is the active menu
	const char *menu;
	/// Array of submenus and callbacks for each choice the user can make
	const struct menu_step_s *choices;
	/// Function to be called when the user selects this item
	menu_func_t func;
} menu_step_t;

#endif
