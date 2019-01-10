#ifndef ANIMSPRITE_H
#define ANIMSPRITE_H

#include <stdarg.h> // va_* macros are defined here

#include "sprite.h"

/** @defgroup AnimSprite AnimSprite
 * @{
 * Functions to animate the game character
 */

/**
 * @brief Struct type for specifying the character animation
 */
typedef struct {
  Sprite *sp;     /**< @brief standard sprite */
  int aspeed;     /**< @brief no. frames per pixmap */
  int cur_aspeed; /**< @brief no. frames left to next change */
  int num_fig;    /**< @brief number of pixmaps */
  int cur_fig;    /**< @brief current pixmap */
  uint8_t **map;  /**< @brief array of pointers to pixmaps */
} AnimSprite;

/**
 * @brief Function to create a AnimSprite data type through the xpms specified in the arguments
 * @param xi, yi Initial coordinates for the AnimSprite
 * @param no_pic Number os xpm_string_t values that will be received in the function
 * @param pic1[] Array of xpm_string_t that speficies the xpm that will be loaded
 * @return Returns a pointer to the AnimSprite struct created
 */
AnimSprite *create_animSprite(uint16_t xi, uint16_t yi, uint8_t no_pic, xpm_string_t pic1[], ...);

/**
 * @brief Function used to update the AnimSprite: changes pictures
 * @param sp Pointer to a AnimSprite struct
 * @return Returns 1 if there was an error. Otherwise returns 0
 */
int animate_animSprite(AnimSprite *sp);

/**
 * @brief Funciton used to free the memory allocated to the entire struct
 * @param sp Pointer to the AnimSprite struct that will be destroyed
 */
void destroy_animSprite(AnimSprite *sp);

/**
 * @brief Function used to determines what is the current picture
 * @param sp AnimSprite pointer
 * @return Returns the cur_fig attribute of the AnimSprite struct 
 */
int get_sprite_number(AnimSprite *sp);

/**
 * @brief Function that swaps the content of two AnimSprite structs
 * @param sp1, sp2 Pointers to 2 AnimSprite structs
 */
void swap_animSprites(AnimSprite *sp1, AnimSprite *sp2);

/**
 * @}
 */

#endif /*ANIMSPRITE_H*/
