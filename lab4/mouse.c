#include <lcom/lcf.h>

#include <minix/syslib.h>
#include <minix/sysutil.h>
#include <stdint.h>

#include "8042.h"
#include "mouse.h"

int hook_id_mouse; //Global variable that will contain the value of hook_id used to subscribe and unsubscribe the interrupts
int hook_id_mouse = 0;

uint8_t mouseByte = 0; //Global variable that will contain a mouse byte

int mouse_subscribe_int(uint8_t *bit_no) { //Function that subscribes mouse interrupts

  hook_id_mouse = (int) *bit_no;
  //Assigning the value of bit_no to the global variable hook_id
  //so that we can preserve bit_no when we call sys_irqsetpolicy

  if (sys_irqsetpolicy(MOUSE_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &hook_id_mouse) != OK) //Subscribing a notification on every interrupt in the input TIMER0_IRQ
    return 1;                                                                          //Value of hook_id will be used later in timer_unsubscribe_int()

  *bit_no = (uint8_t) BIT(*bit_no); //Returning the bit with number bit_no setting to 1

  return 0;
}

int mouse_unsubscribe_int() { //Function that unsubscribes mouse interrupts

  if (sys_irqrmpolicy(&hook_id_mouse) != OK) //Unsubscribing the subscription of the interrupt notification associated with the specified hook_id
    return 1;

  return 0;
}

void(mouse_ih)() {             //Mouse interrupt handler
  mouseByte = readMouseByte(); //Only call a function to read a mouse byte
}

uint8_t readMouseByte() { //Function that reads a byte from the mouse
  uint32_t stat;          //Status
  uint32_t data;          //Variable that will contain a byte from the scancode

  while (1) {

    sys_inb(KBC_CMD_REG, &stat); /*assuming it returns OK*/
    //KBC_CMD_REG = Status Register - 0x64
    /*loop while 8042 output buffer is empty*/
    if ((stat & OBF) && (stat & AUX)) {
      sys_inb(OUT_BUF, &data);              /*assuming it returns OK*/
      if ((stat & (PAR_ERR | TO_ERR)) == 0) //Verify if there is an error
        return (uint8_t) data;
      else
        return LARGEST_NUMBER;
    }

    tickdelay(micros_to_ticks(DELAY_US)); //In interrupts, this instruction won't be executed
  }
}

void parsePacket(struct packet *pp, uint8_t bytes[]) { //Functiona that parses the mouse packet
  //Array bytes of the struct pp will be equal to array bytes
  pp->bytes[0] = bytes[0];
  pp->bytes[1] = bytes[1];
  pp->bytes[2] = bytes[2];
  //mb, rb and lb are booleans that corresponds to bits 2, 1 and 0, respectively, of the first byte
  pp->rb = ((bytes[0] & RB) >> 1);
  pp->mb = ((bytes[0] & MB) >> 2);
  pp->lb = (bytes[0] & LB);

  //Expand second and third byte with signal
  if (bytes[0] & X_SIGN) {
    pp->delta_x = bytes[1] | (LARGEST_NUMBER << 8);
  }
  else {
    pp->delta_x = bytes[1];
  }

  if (bytes[0] & Y_SIGN) {
    pp->delta_y = ((LARGEST_NUMBER << 8) | bytes[2]);
  }
  else {
    pp->delta_y = bytes[2];
  }

  //x_ov and y_ov are booleans that corresponds to bits 6 and 7 of the first byte
  pp->x_ov = ((bytes[0] & X_OV) >> 6);
  pp->y_ov = ((bytes[0] & Y_OV) >> 7);
}

int issueMouseCommand(uint8_t cmd) { //Function that sends a command to mouse
  uint32_t stat;                     //Variable that will contain the status register
  uint32_t ack;                      //Variable that will contain the acknowledgement byte
  do {
    sys_inb(KBC_CMD_REG, &stat); /*assuming it returns OK*/
    //KBC_CMD_REG = Status Register - 0x64

    if (stat & IBF) //Verify if input buffer is full
      continue;

    if (sys_outb(KBC_CMD_REG, WRITE_BYTE_MOUSE) != OK) //Write 0xD4 to kbc command register
      return 1;

    sys_inb(KBC_CMD_REG, &stat); /*assuming it returns OK*/
    //KBC_CMD_REG = Status Register - 0x64

    if (stat & IBF) //Verify if input buffer is full
      continue;

    if (sys_outb(OUT_BUF, cmd) != OK) //Write mouse command to output buffer
      return 1;

    sys_inb(KBC_CMD_REG, &stat); /*assuming it returns OK*/
    //KBC_CMD_REG = Status Register - 0x64

    if ((stat & OBF) && (stat & AUX)) { //Verify if output buffer is full and the byte received corresponds to mouse
      if (sys_inb(OUT_BUF, &ack) != OK) //Read acknowledgement byte
        return 1;

      if (ack == ERROR) //If the acknowledgement byte is 0xFC, occured an error
        return 1;
    }
  } while (ack != ACK); //Continue until the acknowledgement byte is 0xFA

  return 0;
}

int mouseIrqSet(bool enable) { //Function to enable and disable irq line
  if (enable) {
    if (sys_irqenable(&hook_id_mouse) != OK) //Enable irq line
      return 1;
  }
  else {
    if (sys_irqdisable(&hook_id_mouse) != OK) //Disable irq line
      return 1;
  }
  return 0;
}

int writeDafaultCmdByte() {                    //Function to write the deafult command byte
  uint8_t cmb = minix_get_dflt_kbc_cmd_byte(); //Get minix default command byte
  uint32_t stat;                               //Variable that will contain the status register
  while (1) {
    sys_inb(KBC_CMD_REG, &stat); /*assuming it returns OK*/
    //KBC_CMD_REG = Status Register - 0x64

    if (stat & IBF) //Verify if input buffer is full
      continue;

    if (sys_outb(KBC_CMD_REG, WRITE_COM_B) != OK) //Instruction to make possible the writing of the command byte
      return 1;

    sys_inb(KBC_CMD_REG, &stat); /*assuming it returns OK*/
    //KBC_CMD_REG = Status Register - 0x64

    if (stat & IBF) //Verify if input buffer is full
      continue;

    if (sys_outb(OUT_BUF, (uint32_t) cmb) != OK) //Writing the command byte to the output buffer
      return 1;

    break;
  }
  return 0;
}

void mouseDetectEvent(struct packet *pp, enum state_t state) { //Function that detects an event provide from the mouse
  switch (state) {                                             //Verify the last state. Depending on the last state it will react in different ways
    case INIT:                                                 //Initial state
      if (pp->lb && !(pp->mb || pp->rb)) {                     //If only the left button is pressed, event = LB_PRESSED
        m.type = LB_PRESSED;
      }
      else if (pp->mb || pp->rb) { //If some other button is pressed, event = BUTTON_EV
        m.type = BUTTON_EV;
      }
      else { //If no button is pressed, event = MOUSE_MOV
        m.type = MOUSE_MOV;
      }
      break;
    case LLINE:               //Left line of the inverted V
      if (pp->rb || pp->mb) { //If some button is pressed except the left, event = BUTTON_EV
        m.type = BUTTON_EV;
      }
      else if (pp->lb) { //If left button is pressed, event = MOUSE_MOV because lb as already been pressed
        m.type = MOUSE_MOV;
      }
      else { //If no button is pressed, this means that the left has been released, so event = LB_RELEASED
        m.type = LB_RELEASED;
      }
      break;
    case VERTEX:    //Vertex of the inverted V
      if (pp->mb) { //If middle button is pressed, event = MOUSE_EV
        m.type = BUTTON_EV;
      }
      else if (pp->lb) { //If left button is pressed, event = LB_PRESSED
        m.type = LB_PRESSED;
      }
      else if (pp->rb) { //If right button is pressed, event = RB_PRESSED
        m.type = RB_PRESSED;
      }
      else { //If no button is pressed, event = MOUSE_MOV
        m.type = MOUSE_MOV;
      }
      break;
    case RLINE:               //Right line of the inverted V
      if (pp->lb || pp->mb) { //If left or middle button is pressed, event = BUTTON_EV
        m.type = BUTTON_EV;
      }
      else if (pp->rb) { //If only the right button is pressed, event = MOUSE_MOV because rb as already been pressed
        m.type = MOUSE_MOV;
      }
      else { //If no button is pressed, this means that the right has been released, so event = RB_RELEASED
        m.type = RB_RELEASED;
      }
      break;
    default: //Final state, it won't be reached
      m.type = MOUSE_MOV;
      break;
  }

  //Delta x and y of the struct m is equal to the delta x and y of the struct pp
  m.delta_x = pp->delta_x;
  m.delta_y = pp->delta_y;
}
