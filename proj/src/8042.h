#ifndef _LCOM_8042_H_
#define _LCOM_8042_H_

/** @defgroup i8042 i8042
 * @{
 * Constants for programming the 8042 Keyboard and Mouse Controller.
 */

#define BIT(n) (0x01 << (n))

#define KEYBOARD_IRQ 1 /**< @brief Keyboard IRQ line */
#define MOUSE_IRQ 12   /**< @brief Mouse IRQ line */

//KEYBOARD SCANCODES
#define ESC 0x81         /**< @brief ESC key: used to stop the cicle of pollings */
#define UP_ARROW 0x48    /**< @brief Up arrow and numpad 8 */
#define BYTE_SPACE 0x39  /**< @brief space key */
#define BACKSPACE 0x0E   /**< @brief backspace key */
#define BYTE_A 0x1E      /**< @brief A makecode */
#define BYTE_B 0x30      /**< @brief B makecode */
#define BYTE_C 0x2E      /**< @brief C makecode */
#define BYTE_D 0x20      /**< @brief D makecode */
#define BYTE_E 0x12      /**< @brief E makecode */
#define BYTE_F 0x21      /**< @brief F makecode */
#define BYTE_G 0x22      /**< @brief G makecode */
#define BYTE_H 0x23      /**< @brief H makecode */
#define BYTE_I 0x17      /**< @brief I makecode */
#define BYTE_J 0x24      /**< @brief J makecode */
#define BYTE_K 0x25      /**< @brief K makecode */
#define BYTE_L 0x26      /**< @brief L makecode */
#define BYTE_M 0x32      /**< @brief M makecode */
#define BYTE_N 0x31      /**< @brief N makecode */
#define BYTE_O 0x18      /**< @brief O makecode */
#define BYTE_P 0x19      /**< @brief P makecode */
#define BYTE_Q 0x10      /**< @brief Q makecode */
#define BYTE_R 0x13      /**< @brief R makecode */
#define BYTE_S 0x1F      /**< @brief S makecode */
#define BYTE_T 0x14      /**< @brief T makecode */
#define BYTE_U 0x16      /**< @brief U makecode */
#define BYTE_V 0x2F      /**< @brief V makecode */
#define BYTE_W 0x11      /**< @brief W makecode */
#define BYTE_X 0x2D      /**< @brief X makecode */
#define BYTE_Y 0x15      /**< @brief Y makecode */
#define BYTE_Z 0x2C      /**< @brief Z makecode */
#define RIGHT_ARROW 0x4D /**< @brief right arrow and numpad 6 */
#define LEFT_ARROW 0x4B  /**< @brief left arrow and numpad 4 */
#define ENTER 0x1C       /**< @brief Enter key */

#define FIRST_B_SCAN 0xE0 /**< @brief Usual value of the first byte of two byte scancodes */

#define KBC_CMD_REG 0x64 /**< @brief Address of the status register variable and register to write kbc commands */

#define OUT_BUF 0x60 /**< @brief Output Buffer */

#define PAR_ERR BIT(7) /**< @brief Parity error -> bit 7 of the status */
#define TO_ERR BIT(6)  /**< @brief Timeout error -> bit 6 of the status */
#define OBF BIT(0)     /**< @brief Output buffer full - data available for reading -> bit 0 of the status */
#define IBF BIT(1)     /**< @brief Input buffer full - don’t write commands or arguments -> bit 1 of the status */
#define AUX BIT(5)     /**< @brief Auxiliar bit - when set to 1, the byte comes from the mouse */

#define READ_COM_B 0x20       /**< @brief Read Command Byte */
#define WRITE_COM_B 0x60      /**< @brief Write Command Byte */
#define CHECK_KBC 0xAA        /**< @brief Check KBC (Self-test) */
#define CHECK_KBD_INT 0xAB    /**< @brief Check Keyboard Interface */
#define DIS_KBD_INT 0xAD      /**< @brief Disable KBD Interface */
#define EN_KBD_INT 0xAE       /**< @brief Enable KBD Interface */
#define WRITE_BYTE_MOUSE 0xD4 /**< @brief Write byte to mouse */

#define DELAY_US 20000 /**< @brief Argument of tickdelay function */

#define LARGEST_NUMBER 0xFF /**< @brief FF is the largest binary number */

#define BREAK_CODE BIT(7) /**< @brief If the bit 7 of the scancode is set to 1 it means that is a makecode */

#define ENABLE_INT_KBD BIT(0)        /**< @brief Value to use in the command byte to enable keyboard interrupts */
#define ENABLE_INT_MOUSE BIT(1)      /**< @brief Value to use in the command byte to enable mouse interrupts */
#define DISABLE_MOUSE BIT(5)         /**< @brief Value to use in the command byte to disable mouse */
#define DISABLE_KBD_INTERFACE BIT(4) /**< @brief Value to use in the command byte to disable keyboard interface */

#define LB BIT(0)     /**< @brief Left button bit */
#define RB BIT(1)     /**< @brief Right button bit */
#define MB BIT(2)     /**< @brief Middle button bit */
#define X_OV BIT(6)   /**< @brief x overflow bit */
#define Y_OV BIT(7)   /**< @brief y overflow bit */
#define X_SIGN BIT(4) /**< @brief x sign bit */
#define Y_SIGN BIT(5) /**< @brief y sign bit */

//PS/2 Mouse Commands
#define READ_DATA 0xEB           /**< @brief Send data packet request */
#define STREAM_MODE 0xEA         /**< @brief Sed data on events */
#define ENABLE_DATA_REPORT 0xF4  /**< @brief In stream mode only */
#define DISABLE_DATA_REPORT 0xF5 /**< @brief Disable (Data Reporting) */
#define REMOTE_MODE 0xF0         /**< @brief Set remote mode - Send data on request only */

//Acknowledgement byte values
#define ACK 0xFA   /**< @brief Acknowledgement byte: it means that the command was succesfully sent to the mouse */
#define NACK 0xFE  /**< @brief NACK byte: it means that occurred an error on senting the command to the mouse, but you can try again */
#define ERROR 0xFC /**< @brief ERROR byte: like NACK, but this time you can't continue sending commands to mouse */

//Values passed to function mouseIrqSet to disable and enable irq line
#define DISABLE_IRQ_LINE 0 /**< @brief MACRO passed to mouseIrqSet so that it can disable mouse irq line */
#define ENABLE_IRQ_LINE 1  /**< @brief MACRO passed to mouseIrqSet so that it can enable mouse irq line */

/**
 * @}
 */

#endif /* _LCOM_8042_H */
