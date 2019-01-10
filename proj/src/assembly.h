#ifndef ASSEMBLY_H
#define ASSEMBLY_H

/** @defgroup Assembly Assembly
 * @{
 * Interrupt handlers in assembly
 */

/**
 * @brief Handle timer interrupts in assembly
 */
void timer_asm_ih(void);

/**
 * @brief Handle keyboard interrupts in assembly
 */
void kbd_asm_ih(void);

/**
 * @brief Handle mouse interrupts in assembly
 */
void mouse_asm_ih(void);

/**
 * @}
 */

#endif /*ASSEMBLY_H*/
