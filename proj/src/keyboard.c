#include <lcom/lcf.h>

#include <minix/syslib.h>
#include <minix/sysutil.h>
#include <stdint.h>

#include "8042.h"
#include "keyboard.h"

int hook_id_kbd; //Global variable that will contain the value of hook_id used to subscribe and unsubscribe the interrupts

uint8_t code = 0;  //Scancode

int(kbd_subscribe_int)(uint8_t *bit_no) {

  hook_id_kbd = (int) *bit_no;
  //Assigning the value of bit_no to the global variable hook_id
  //so that we can preserve bit_no when we call sys_irqsetpolicy

  if (sys_irqsetpolicy(KEYBOARD_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &hook_id_kbd) != OK) //Subscribing a notification on every interrupt in the input TIMER0_IRQ
    return 1;                                                                       //Value of hook_id will be used later in timer_unsubscribe_int()

  *bit_no = (uint8_t) BIT(*bit_no); //Returning the bit with number bit_no setting to 1

  return 0;
}

int(kbd_unsubscribe_int)() {

  if (sys_irqrmpolicy(&hook_id_kbd) != OK) //Unsubscribing the subscription of the interrupt notification associated with the specified hook_id
    return 1;

  return 0;
}

