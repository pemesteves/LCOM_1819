#include <lcom/lcf.h>

#include <minix/syslib.h>
#include <minix/sysutil.h>
#include <stdint.h>

#include "8042.h"
#include "keyboard.h"

int hook_id_kbd; //Global variable that will contain the value of hook_id used to subscribe and unsubscribe the interrupts

uint32_t counterSysIn = 0; //Number of sys_inb calls
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

void (kbc_ih)() {
  
  uint32_t data = readScanCode(); //Variable that will contain a byte from the scancode

  if (data == LARGEST_NUMBER) //If data is -1, it has an error in the reading of the scancode, so code = 0
    code  = 0;
  else
    code = (uint8_t)data; //If there is no error, code will have the scancode
   
}

uint8_t readScanCode(){

  uint32_t stat; //Status
  uint32_t data; //Variable that will contain a byte from the scancode

  while( 1 ) {

   sys_inb_cnt(KBC_CMD_REG, &stat); /*assuming it returns OK*/
   //KBC_CMD_REG = Status Register - 0x64
    /*loop while 8042 output buffer is empty*/
   if( stat & OBF ) {
     sys_inb_cnt(OUT_BUF, &data); /*assuming it returns OK*/
     if ( (stat & (PAR_ERR | TO_ERR)) == 0 ) //Verify if there is an error
       return data;
     else
        return LARGEST_NUMBER;
    }
   tickdelay(micros_to_ticks(DELAY_US)); //In interrupts, this instruction won't be executed
  }
}

void enableInterrupts(){ 
  sys_outb(KBC_CMD_REG, READ_COM_B); //Instruction to make possible the reading of the command byte

  uint32_t cmb;
  sys_inb_cnt(OUT_BUF, &cmb); //Reading the command byte
  
  sys_outb(KBC_CMD_REG, WRITE_COM_B); //Instruction to make possible the writing of the command byte


  cmb |= ENABLE_INT_KBD; //Enabling the keyboard interrupts

  sys_outb(OUT_BUF, cmb); //Writing the command byte to the output buffer 
}

int (sys_inb_cnt)(port_t port, uint32_t *byte) {
  counterSysIn++; //Count the sys_inb call
  if (sys_inb(port, byte) != OK) //Call to sys_inb function
    return 1;
  return 0;
}
