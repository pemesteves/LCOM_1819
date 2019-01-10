#ifndef SPRITE_H
#define SPRITE_H

/** @defgroup Sprite Sprite
 * @{
 * Functions to represent, move and detect collisions with game objects
 */

/**
 * @brief Struct type for specifying a Sprite
 */
typedef struct {
  int x, y;           /*!<  current position */
  int width, height;  /*!<  dimensions */
  int xspeed, yspeed; /*!<  current speed */
  uint8_t *map;       /*!<  the pixmap */
} Sprite;

/** 
 * @brief Creates a new sprite with pixmap "pic", with specified position (within the screen limits) and speed;
 *         Does not draw the sprite on the screen
 *
 * @param pic1[] Xpm that will be load and converted to a Sprite struct
 * @param x, y Initial coordinates of the Sprite
 * @param xspeed, yspeed Initial speed of the Sprite
 * 
 * @return Returns NULL on invalid pixmap.
 */
Sprite *create_sprite(const xpm_string_t pic[], int x, int y, int xspeed, int yspeed);

/**
 * @brief Destroys a Sprite struct
 * @param sp Pointer to the struct that will be destroyed
 */
void destroy_sprite(Sprite *sp);

/**
 * @brief Draws a Sprite on the screen
 * @param sp Pointer to the Sprite struct that will be draw
 * @return Return 0 upon success and non-zero otherwise
 */
int draw_sprite(Sprite *sp);

/**
 * @brief State Machine for the character jump (changed the character position in y)
 * @param sp Pointer to the jump Sprite
 * @return Return 0 upon success and non-zero otherwise
 */
int makeJump(Sprite *sp);

/**
 * @brief Clear the screen with the predefined background
 * @param isDay If it's day, isDay is true and false otherwise
 * @param first_time_drawing_background If this is true, background pointer will be initialize. Otherwise it will be copied to the second buffer
 * @return Return 0 upon success and non-zero otherwise
 */
int clear_game_screen(bool isDay, bool first_time_drawing_background);

/**
 * @brief Function to get the dead Sprite corresponding to the actual character animation (state machine)
 * @param position Indicates what is the position of the character (run, jump, start)
 * @param sp Sprite of the actual animation
 * @param noTongue Indicates if the character has no tongue outside its mouth
 * @return Returns a pointer to a Sprite struct with the dead Sprite
 */
Sprite *get_dead_sprite(int position, Sprite *sp, bool noTongue);

/**
 * @brief Function that reinitialize a global variable that maintains the jump position
 */
void reinitialize_jumpPosition();

/**
 * @brief Swaps the content of two sprites
 * @param sp1, sp2 Sprites that will be swapped
 */
void swap_sprites(Sprite *sp1, Sprite *sp2);

/**
 * @brief Updates the enemies position
 * @param num_enemies Number of enemies in the array
 * @param enemies[] Array of enemies
 */
void update_enemies_positions(uint8_t num_enemies, Sprite* enemies[]);

/**
 * @brief Draws all the enemies on the screen
 * @param num_enemies Number of enemies in the array
 * @param enemies[] Array of enemies
 * @return Return 0 upon success and non-zero otherwise
 */
int draw_enemies(uint8_t num_enemies, Sprite *enemies[]);

/**
 * @brief Put a enemy on tha enemies array
 * @param num_enemies Number of enemies in the array
 * @param enemies[] Array of enemies
 * @param new_enemy Enemy that will be putted in the array if there is space
 */
void put_enemie_on_position(uint8_t num_enemies, Sprite *enemies[], Sprite* new_enemy);

/**
 * @brief Checks if there was a collision between the character and the enemies
 * @param sp Character Sprite
 * @param num_enemies Number of enemies in the array
 * @param enemies[] Array of enemies
 * @param jumping Indicates if the character is jumping
 * @return Return 0 if there was no collision and non-zero otherwise
 */
int check_collisions(Sprite *sp, uint8_t num_enemies, Sprite *enemies[], bool jumping);

/**
 * @brief Checks if there was a collision between the cursor and a rectangle in the menu
 * @param cursor Cursor Sprite
 * @param rectangle Rectangle Sprite
 * @return Return 0 if there was no collision and non-zero otherwise
 */
int check_cursor_collisions(Sprite *cursor, Sprite *rectangle);

/**
 * @brief Destroys all the enemies
 * @param num_enemies Number of enemies of the array
 * @param enemies[] Array of enemies
 */
void destroy_enemies(uint8_t num_enemies, Sprite *enemies[]);

/**
 * @}
 */

#endif /*SPRITE_H*/
