#include <lcom/lcf.h>
#include <lcom/timer.h>

#include "8042.h"
#include "keyboard.h"
#include "i8254.h"
#include <stdbool.h>
#include <stdint.h>

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need/ it]
  lcf_trace_calls("/home/lcom/labs/lab3/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab3/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int (kbd_test_scan)(bool assembly) {
  counterSysIn = 0; //Number of sys_inb calls is zero in the beginning of the function
  uint8_t irq_set = 3; //This is the number we use when we subscribe the interrupt of the keyboard
  if (kbd_subscribe_int(&irq_set)) { //Subscribing keyboard interrupts
    kbd_print_no_sysinb(counterSysIn);
    return 1;
  }

  int ipc_status, r;
  message msg;
  bool make = false; //Boolean to decide if the scancode is a break or a makecode
  uint8_t size = 0; //Variable with the number of bytes of the scancode

  uint8_t bytes[2]; //Array that will contain the bytes of the scancode

  while (code != ESC) { //The cicle ends after the time we specify when we call the function
    code = 0;
    /*You may want to use a different condition
        Get a request message.*/
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) { /* received notification */
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:                             /* hardware interrupt notification */
          if (msg.m_notify.interrupts & irq_set) { /* subscribed keyboard interrupt */
            if (assembly)
              kbc_asm_ih(); //Handles interrupts in assembly IA-32
            else 
              kbc_ih(); //Handles interrupts in C
          }
      
          break;
        default:
          break; /* no other notifications expected: do nothing */
      }
    }
    
    size++; //In this point we received a byte from the scancode so size will have another byte 

    if (code == FIRST_B_SCAN){ //If the byte is 0xE0, it will receive a two bytes scancode
      code = 0;
      continue;
    }
    else if (code == 0){ //If code is zero, an error has occurred
      size = 0;
      continue;
    }
    else if (size > 0 && size <= 2) //There aren't codes with more than two bytes: a scancode can only have 1 or 2 bytes
    {
      if (code & BREAK_CODE) //If the bit 7 is 1 it is a breakcode
        make = false;
      else
        make = true;
        
      //Decide if the array will have 1 or 2 bytes
      if (size == 1){ 
        bytes[0] = code;
        kbd_print_scancode(make, size, bytes);
      }
      else{
        bytes[0] = FIRST_B_SCAN;
        bytes[1] = code;
        kbd_print_scancode(make, size, bytes);
      } 
    }
    size = 0;
  }

  if (kbd_unsubscribe_int()) { //Unsubscribing keyboard interrupts
    kbd_print_no_sysinb(counterSysIn);
    return 1;
  }

  if (!assembly) //We want to print the number of sys_inb calls only in C interrupt handler
    kbd_print_no_sysinb(counterSysIn); //Print the number of sys_inb calls
  
  return 0;
}


int(kbd_test_poll)() {
  counterSysIn = 0; //Number of sys_inb calls is zero in the beginning of the function
  bool make = false; //Boolean to decide if the scancode is a break or a makecode
  uint8_t size = 0; //Variable with the number of bytes of the scancode
  uint8_t bytes[2]; //Array that will contain the bytes of the scancode

  while (code != ESC) { //The cicle ends after the time we specify when we call the function

    code = readScanCode(); //Reading the scancode

    size++; //Incrementing 1 byte in the size

    if (code == FIRST_B_SCAN){ //If the byte is 0xE0, it will receive a two bytes scancode
      code = 0;
      continue;
    }
    else if (code == 0){ //If code is zero, an error has occurred
      size = 0;
      continue;
    }
    else if (size > 0 && size <= 2) //There aren't codes with more than two bytes: a scancode can only have 1 or 2 bytes
    {
      if (code & BREAK_CODE) //If the bit 7 is 1 it is a breakcode
        make = false;
      else
        make = true;
        
      //Decide if the array will have 1 or 2 bytes
      if (size == 1){
        bytes[0] = code;
        kbd_print_scancode(make, size, bytes);
      }
      else{
        bytes[0] = FIRST_B_SCAN; 
        bytes[1] = code;
        kbd_print_scancode(make, size, bytes);
      } 
    }
    size = 0;
  }

  enableInterrupts(); //Enabling interrupts so that keyboard can works after we run this function

  kbd_print_no_sysinb(counterSysIn); //Print the number of sys_inb calls
  return 0;
}


int(kbd_test_timed_scan)(uint8_t n) {
  if (n < 0) //Time is always positive
    return 1; 
  
  //If n is 0, the interrupts will be subscribed and unsubscribe without being handled

  //The number we use to subscribe timer interrupts and keyboard interrupts has to be different
  uint8_t irq_set_time = 2; //This is the number we use when we subscribe the interrupt of the timer
  if (timer_subscribe_int(&irq_set_time)) //Subscribing timer interrupts
    return 1;

  uint8_t irq_set_kbd = 3; //This is the number we use when we subscribe the interrupt of the keyboard
  if (kbd_subscribe_int(&irq_set_kbd)) { //Subscribing keyboard interrupts
    return 1;
  }

  int ipc_status, r;
  message msg;
  bool make = false; //Boolean to decide if the scancode is a break or a makecode
  uint8_t size = 0; //Variable with the number of bytes of the scancode
  uint8_t bytes[2]; //Array that will contain the bytes of the scancode

  while (code != ESC && counter/60 < n) { //The cicle ends after the time we specify when we call the function
    code = 0;
    /*You may want to use a different condition
        Get a request message.*/
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) { /* received notification */
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:                             /* hardware interrupt notification */
          if (msg.m_notify.interrupts & irq_set_kbd) { /* subscribed keyboard interrupt */
              kbc_ih(); //Handles interrupts in C
              counter = 0;
          }
          else if (msg.m_notify.interrupts & irq_set_time) { /* subscribed interrupt */

            timer_int_handler(); //Counting the number of interruptions

            continue; //We continue to the next iteration because we don't have any scancode
          }
          break;
        default:
          break; /* no other notifications expected: do nothing */
      }
    }
    
    size++;

    if (code == FIRST_B_SCAN){ //If the byte is 0xE0, it will receive a two bytes scancode
      code = 0;
      continue;
    }
    else if (code == 0){ //If code is zero, an error has occurred
      size = 0;
      continue;
    }
    else if (size > 0 && size <= 2) //There aren't codes with more than two bytes: a scancode can only have 1 or 2 bytes
    {
      if (code & BREAK_CODE) //If the bit 7 is 1 it is a breakcode
        make = false;
      else
        make = true;
        
      //Decide if the array will have 1 or 2 bytes  
      if (size == 1){
        bytes[0] = code;
        kbd_print_scancode(make, size, bytes);
      }
      else{
        bytes[0] = FIRST_B_SCAN;
        bytes[1] = code;
        kbd_print_scancode(make, size, bytes);
      } 
    }
    size = 0;
  }
  
  if (timer_unsubscribe_int()) //Unsubscribing timer interrupts
    return 1;

  if (kbd_unsubscribe_int()) //Unsubscribing keyboard interrupts
    return 1;

  return 0;
}
