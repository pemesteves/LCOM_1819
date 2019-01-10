#ifndef RTC_H
#define RTC_H

/** @defgroup RTC RTC
 * @{
 * Functions to manipulate the Real Time Clock
 */

/**
 * @brief Checks if the registers values are updating
 * @return Returns 0 if it's not updating and non-zero otherwise
 */
int updating_registers();

/**
 * @brief Function to get the value in a register
 * @param port Register that will be accessed
 * @return Returns the register value, probably in bcd
 */
uint8_t get_register_value(uint8_t port);

/**
 * @brief Checks if the values read are in bcd
 * @return Returns 1 if the values are in bcd and 0 if they are in binary
 */
int is_BCD();

/**
 * @brief Converts a bcd value into binary
 * @param bcd_value Value that will be converted in binary
 * @return Returns the value in binary
 */
uint8_t convert_to_binary(uint8_t bcd_value);

/**
 * @brief Function that gives the last hour of the day depending on the month that the player is playing
 * @param month Month when the player is playing
 * @return Returns the starting hour of night
 */
uint8_t get_last_hour_day(uint8_t month);

//MACROS FOR RTC

#define UIP (0x01 << 7) /**< @brief UIP bit */ 
#define DM (0x01 << 2) /**< @brief DM bit */

#define RTC_MAIN_REGISTER 0x70 /**< @brief RTC register that is used to decide what will be the accessed register*/
#define RTC_READ_WRITE_PORT 0x71 /**< @brief RTC register that is used to read or write the data */
#define REGISTER_A 0x0A /**< @brief Register A of RTC: accessed to know if the registers are updating*/
#define REGISTER_B 0x0B /**< @brief Register B of RTC: accessed to know if the value of the registers is BCD or binary */
#define SECONDS 0x00 /**< @brief Seconds register */
#define MINUTES 0x02 /**< @brief Minutes register */
#define HOUR 0x04 /**< @brief Hour register */
#define WEEK_DAY 0x06 /**< @brief Week day register */
#define MONTH_DAY 0x07 /**< @brief Month day register */
#define MONTH 0x08 /**< @brief Month register */
#define YEAR 0x09 /**< @brief Year register */

/**
 * @}
 */

#endif /*RTC_H*/
