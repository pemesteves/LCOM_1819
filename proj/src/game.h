#ifndef GAME_H
#define GAME_H

/** @defgroup Game Game
 * @{
 * Functions to run the game and show it to the user
 */

/**
 * @brief In this function is the game: the raccoon running 
 * @param xi, yi Initial coordinates of the character
 * @param fr_rate Frequency used in timer 0
 * @return Returns 1 if there was an error. Otherwise returns 0
 */
int game(uint16_t xi, uint16_t yi, uint8_t fr_rate);

/**
 * @brief This function is used to draw the game background (sky, grass and ground)
 * @param isDay This parameter determines if the color of the sky is light or dark blue
 * @param first_time_drawing_background If this is true, background pointer will be initialize. Otherwise it will be copied to the second buffer
 * @return Returns 1 if there was an error. Otherwise returns 0
 */
int drawBackground(bool isDay, bool first_time_drawing_background);

/**
 * @brief This function is used to allocate memory to the game background pointer
 */
void game_background_alloc();

/**
 * @brief Function that draws the score in the game screen
 * @param score Actual score of the player
 * @return Returns 1 if there was an error. Otherwise returns 0
 */ 
int draw_score(uint32_t score);

/**
 * @brief Function to free the memory allocated to the game background
 */
void free_game_background();

/**
 * @}
 */

#endif /*GAME_H*/
