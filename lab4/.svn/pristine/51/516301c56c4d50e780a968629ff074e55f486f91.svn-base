#include <lcom/lcf.h>
#include <lcom/timer.h>
#include <minix/syslib.h>
#include <stdint.h>

#include "i8254.h"

int hook_id; //Global variable that will contain the value of hook_id used to subscribe and unsubscribe the interrupts

uint32_t counter = 0; //global counter variable

int(timer_set_frequency)(uint8_t timer, uint32_t freq) {
  if (freq < TIMER_MIN_FREQ || freq > TIMER_FREQ) //If the frequency is above 1193182 the division will be 0
    return 1;                                     //Else if the frequency is under 19 it will make an overflow when we divide the TIMER_FREQ by this; so, to prevent further errors, it will stop here

  uint8_t st; //This variable will have the Read-Back Status

  timer_get_conf(timer, &st); //Getting the Read-Back Status

  uint8_t controlWord = st & (BIT(3) | TIMER_SQR_WAVE | TIMER_BCD); //Preserving the 4 lsb of st
  //controlWord will have the BCD and Counting Mode from st

  controlWord |= TIMER_LSB_MSB; //Initialize LSB first and MSB afterwards

  uint16_t countValue = TIMER_FREQ / freq; //Getting the counting value
  uint8_t msb, lsb;
  util_get_LSB(countValue, &lsb); //Getting the value of LSB
  util_get_MSB(countValue, &msb); //Getting the value of MSB

  switch (timer) { //Here we write the control word to the control port and the lsb and msb to the timer port
    case 0:
      controlWord |= TIMER_SEL0; //Doesn't do nothing if the timer is 0, because the two msb from cW are already 0
      if (sys_outb(TIMER_CTRL, controlWord) != OK)
        return 1;
      if (sys_outb(TIMER_0, lsb) != OK)
        return 1;
      if (sys_outb(TIMER_0, msb) != OK)
        return 1;
      break;
    case 1:
      controlWord |= TIMER_SEL1; //Selecting Timer 1
      if (sys_outb(TIMER_CTRL, controlWord) != OK)
        return 1;
      if (sys_outb(TIMER_1, lsb) != OK)
        return 1;
      if (sys_outb(TIMER_1, msb) != OK)
        return 1;
      break;
    case 2:
      controlWord |= TIMER_SEL2; //Selecting Timer 2
      if (sys_outb(TIMER_CTRL, controlWord) != OK)
        return 1;
      if (sys_outb(TIMER_2, lsb) != OK)
        return 1;
      if (sys_outb(TIMER_2, msb) != OK)
        return 1;
      break;
    default:    //There are only 3 timers (timers 0, 1 and 2)
      return 1; //Occurred an error, so returns anything but 0
  }

  return 0;
}

int(timer_subscribe_int)(uint8_t *bit_no) {

  hook_id = (int) *bit_no;
  //Assigning the value of bit_no to the global variable hook_id
  //so that we can preserve bit_no when we call sys_irqsetpolicy

  if (sys_irqsetpolicy(TIMER0_IRQ, IRQ_REENABLE, &hook_id) != OK) //Subscribing a notification on every interrupt in the input TIMER0_IRQ
    return 1;                                                     //Value of hook_id will be used later in timer_unsubscribe_int()

  *bit_no = (uint8_t) BIT(*bit_no); //Returning the bit with number bit_no setting to 1

  return 0;
}

int(timer_unsubscribe_int)() {

  if (sys_irqrmpolicy(&hook_id) != OK) //Unsubscribing the subscription of the interrupt notification associated with the specified hook_id
    return 1;

  return 0;
}

void(timer_int_handler)() {
  counter++; //Incrementing the global variable counter that counts the number of interruptions
}

int(timer_get_conf)(uint8_t timer, uint8_t *st) {
  if (timer > 2)
    return 1;
  else {
    uint8_t readBack = TIMER_RB_CMD | TIMER_RB_COUNT_; //Read-Back Command without the bits of the timer

    uint32_t rBStatus; //Temporary variable that will contains the Read-Back Status after sys_inb

    if (timer == 0) {
      readBack |= TIMER_RB_SEL(0); //Selecting the timer for Read-Back Command
      if (sys_outb(TIMER_CTRL, readBack) != OK)
        return 1;
      if (sys_inb(TIMER_0, &rBStatus) != OK)
        return 1;
    }
    else if (timer == 1) {
      readBack |= TIMER_RB_SEL(1); //Selecting the timer for Read-Back Command
      if (sys_outb(TIMER_CTRL, readBack) != OK)
        return 1;
      if (sys_inb(TIMER_1, &rBStatus) != OK)
        return 1;
    }
    else {
      readBack |= TIMER_RB_SEL(2); //Selecting the timer for Read-Back Command
      if (sys_outb(TIMER_CTRL, readBack) != OK)
        return 1;
      if (sys_inb(TIMER_2, &rBStatus) != OK)
        return 1;
    }

    *st = (uint8_t) rBStatus; //Passing the Read-Back Status to the st variable so that it can be returned

    return 0;
  }
}

int(timer_display_conf)(uint8_t timer, uint8_t conf, enum timer_status_field field) {
  //The argument conf is assumed to be the configuration of the timer returned by the read-back command -> st from the last function

  union timer_status_field_val configuration;

  switch (field) {
    case all:
      configuration.byte = conf; //configuration will have the conf byte
      break;
    case initial: //Counter Initialization
    {
      uint8_t counterInit = (conf & TIMER_LSB_MSB) >> 4;

      switch (counterInit) {
        case 1: //If the counter initialization is 01b, it corresponds to LSB
          configuration.in_mode = LSB_only;
          break;
        case 2: //If the counter initialization is 10b, it corresponds to MSB
          configuration.in_mode = MSB_only;
          break;
        case 3: //If the counter initialization is 11b, it corresponds to LSB and MSB
          configuration.in_mode = MSB_after_LSB;
          break;
        default: //here the default is considered to be invalid
          configuration.in_mode = INVAL_val;
      }
      break;
    }
    case mode: //Programmed Mode
    {
      uint8_t mode = (conf & (TIMER_SQR_WAVE | BIT(3))) >> 1; //We want to preserve bits 1, 2 and 3 (TIMER_SQR_WAVE corresponds to bit 1 and 2)
      if (mode > 5)
        mode -= 4; //Mode 6 and 7 don't exist -> they are mode 2 and 3 respectively
      configuration.count_mode = mode;
      break;
    }
    case base:
      configuration.bcd = conf & TIMER_BCD; //Preserve bit 0 of conf
      break;
    default:
      return 1; //Occurred an error, so returns anything but 0
  }

  timer_print_config(timer, field, configuration);

  return 0;
}
