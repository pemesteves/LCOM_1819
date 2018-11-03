#pragma once

/** @defgroup lab2 lab2
 * @{
 *
 * Test and utility functions for the Timer's lab
 */

/**
 * @brief Tests display of timer config
 *
 * Just calls timer_get_conf() followed by timer_display_conf()
 *
 * @param timer Timer whose config to read (Ranges from 0 to 2)
 * @param field  Configuration field to display
 * @return Return 0 upon success and non-zero otherwise
 */
int timer_test_read_config(uint8_t timer,
                           enum timer_status_field conf);

/**
 * @brief Tests change of Timer O interrupt frequency
 *
 * Programs Timer 0 to generate interrupts with input frequency
 *
 * @param timer Timer whose time-base should be changed (Ranges from 0 to 2)
 * @param freq Frequency of interrupts to generate
 * @return Return 0 upon success and non-zero otherwise
 */
int timer_test_time_base(uint8_t timer, uint32_t freq);

/**
 * @brief Tests Timer 0 interrupt handling
 *
 * Subscribes Timer 0 interrupts and prints a message once
 *  per second for the specified time interval
 *
 * @param time Length of time interval while interrupts are subscribed
 * @return Return 0 upon success and non-zero otherwise
 */
int timer_test_int(uint8_t time);

/**
 * @brief Returns the LSB of a 2 byte integer
 *
 * @param val input 2 byte integer
 * @param lsb address of memory location to be updated with val's LSB
 * @return Return 0 upon success and non-zero otherwise
 */
int util_get_LSB(uint16_t val, uint8_t *lsb);

/**
 * @brief Returns the MSB of a 2 byte integer
 *
 * @param val input 2 byte integer
 * @param msb address of memory location to be updated with val's MSB
 * @return Return 0 upon success and non-zero otherwise
 */
int util_get_MSB(uint16_t val, uint8_t *msb);
