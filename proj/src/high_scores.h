#ifndef HIGH_SCORES_H
#define HIGH_SCORES_H

/** @defgroup Highscores Highscores
 * @{
 * Functions to deal with the scores and save them in a file
 */

/**
 * @brief Function that inserts a score in the player high_scores array
 * @param score Score of the player in that run
 * @param numElem Number of elements of the array(10)
 * @param high_scores[] Array with the previous high scores and where the score will be inserted
 */
void put_highscore(uint32_t score, uint8_t numElem, uint32_t high_scores[]);

/**
 * @brief Function that update the game high scores and sava them in a file
 * @param numElem Number of elements of the array high_scores
 * @param high_scores[] Array with the player high scores
 * @param name_size Size of the string with the player name
 * @param player_name[] String with player name
 */
void save_high_scores(uint8_t numElem, uint32_t high_scores[], uint8_t name_size, char player_name[]);

/**
 * @brief Function that draw the high scores in the high scores section of the menu
 * @param firstTime If it is true, the function is called for the first time
 */
void draw_high_scores(bool firstTime);

/**
 * @brief Function to allocate memory to high_scores_screen pointer
 */
void alloc_high_scores();

/**
 * @brief Fucnction to free the high scores pointer content
 */
void free_high_scores();

/**
 * @}
 */

#endif /*HIGH_SCORES_H*/
