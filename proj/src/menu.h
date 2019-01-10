#ifndef MENU_H
#define MENU_H

#include "sprite.h"

/** @defgroup Menu Menu
 * @{
 * Functions to deal with the game menu
 */

 /**
  * @brief In this function is the menu and all the subsections of it
  * @return Returns 1 if there was an error. Otherwise returns 0
  */
int menu();

/**
 * @brief Function to get the menu background
 * @return Returns a pointer to menu background allocated memory
 */
char *get_menu_background();

/**
 * @brief Allocates memory to the menu background
 */
void menu_background_alloc();

/**
 * @brief Free memory allocated for menu background
 */
void free_menu_background();

/**
 * @brief Function to get the sprite corresponding to a given character
 * @param character Character that needs the sprite
 * @return Returns NULL if the xpm doesn't exist. Otherwise returns a pointer to a Sprite struct with the character xpm
 */
Sprite *get_character(char character);

/** @brief Enumeration that represents the states of the menu mouse cursor*/
enum cursor_state {
	/** @brief Initial state */
	INITIAL,
	/** @brief When the cursor is above the box */
	IN_BOX,
	/** @brief Final state: only one left click */
	FINAL
};

/**
 * @}
 */

#endif /*MENU_H*/
