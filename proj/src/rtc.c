#include <lcom/lcf.h>
#include <minix/syslib.h>
#include <stdint.h>

#include "rtc.h"

int updating_registers() { //Checks if the registers values are updating
  if (sys_outb(RTC_MAIN_REGISTER, REGISTER_A) != OK) 
    return -1;

  uint32_t registerA_value = 0;

  if (sys_inb(RTC_READ_WRITE_PORT, &registerA_value) != OK) //Read Register A
    return -1;

  return (registerA_value & UIP) >> 7;
}

uint8_t get_register_value(uint8_t port) { //Function to get the value in a register
  if (sys_outb(RTC_MAIN_REGISTER, port) != OK)
    return -1;

  uint32_t register_value = 0;

  if (sys_inb(RTC_READ_WRITE_PORT, &register_value) != OK) //Read port value
    return -1;

  return (uint8_t) register_value;
}

int is_BCD() { //Checks if the values read are in bcd
  if (sys_outb(RTC_MAIN_REGISTER, REGISTER_B) != OK)
    return -1;

  uint32_t registerB_value = 0;

  if (sys_inb(RTC_READ_WRITE_PORT, &registerB_value) != OK) //Read Register B
    return -1;

  if ((registerB_value & DM) == 0) //is BCD
    return 1;
  else
    return 0;
}

uint8_t convert_to_binary(uint8_t bcd_value) { //Converts a bcd value into binary
  return (10 * ((bcd_value & 0xF0) >> 4)) + (bcd_value & 0x0F);
}

uint8_t get_last_hour_day(uint8_t month) { //Function that gives the last hour of the day depending on the month that the player is playing
  uint8_t last_hour_day = 0;

  switch (month) { 
    case 12: 
      last_hour_day = 17;
      break;
    case 1:
    case 11:
      last_hour_day = 18;
      break;
    case 2:
    case 10:
      last_hour_day = 19;
      break;
    case 3:
    case 4:
    case 5:
    case 9:
      last_hour_day = 20;
      break;
    default:
      last_hour_day = 21;
      break;
  }

  return last_hour_day;
}

