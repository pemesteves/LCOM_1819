#pragma once

/** @defgroup Mouse Mouse
 * @{
 * Functions to manipulate the mouse
 */

/**
 * @brief Subscribes and enables Mouse interrupts
 *
 * @param bit_no address of memory to be initialized with the
 *         bit number to be set in the mask returned upon an interrupt
 * @return Return 0 upon success and non-zero otherwise
 */
int mouse_subscribe_int(uint8_t *bit_no); 

/**
 * @brief Unsubscribes Mouse interrupts
 *
 * @return Return 0 upon success and non-zero otherwise
 */
int mouse_unsubscribe_int(); 

/**
 * @brief Parses a mouse packet
 * @param pp Pointer to a struct where the packet will be parsed
 * @param bytes[] Mouse packet (bytes read form the kbc)
 */
void parsePacket(struct packet *pp, uint8_t bytes[]); 

/**
 * @brief Sends a command to mouse
 * @param cmd Command that will the send to the mouse
 * @return Return 0 upon success and non-zero otherwise
 */
int issueMouseCommand(uint8_t cmd); 

/**
 * @brief Enable and disable the mouse irq line
 * @param enable If it is true, it will enable the irq line. Otherwise it will disable it
 * @return  Return 0 upon success and non-zero otherwise
 */
int mouseIrqSet(bool enable); 

/** @brief Extern variable that contains the byte from the mouse */
extern uint8_t mouseByte; 


/**
 * @}
 */
