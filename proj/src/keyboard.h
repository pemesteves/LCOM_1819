#pragma once

/** @defgroup Keyboard Keyboard
 * @{
 * Functions to manipulate the keyboard
 */

/**
 * @brief Subscribes and enables Keyboard interrupts
 *
 * @param bit_no address of memory to be initialized with the
 *         bit number to be set in the mask returned upon an interrupt
 * @return Return 0 upon success and non-zero otherwise
 */
int kbd_subscribe_int(uint8_t *bit_no); 

/**
 * @brief Unsubscribes Keyboard interrupts
 *
 * @return Return 0 upon success and non-zero otherwise
 */
int kbd_unsubscribe_int(); 

extern uint8_t code; /**< @brief Global variable to save scancodes */

/**
 * @}
 */
