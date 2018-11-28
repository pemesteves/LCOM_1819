#include <lcom/lcf.h>
#include <lcom/timer.h>

#include <stdint.h>
#include <stdio.h>

// Any header files included below this line should have been created by you
#include "8042.h"
#include "i8254.h"
#include "mouse.h"

struct mouse_ev m;

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need/ it]
  lcf_trace_calls("/home/lcom/labs/lab4/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab4/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(mouse_test_packet)(uint32_t cnt) {
  uint8_t irq_set = 4; //This is the number we use when we subscribe the interrupt of the mouse

  if (mouse_subscribe_int(&irq_set)) { //Subscribing mouse interrupts
    return 1;
  }

  if (mouseIrqSet(DISABLE_IRQ_LINE)) //Disable irq line
    return 1;

  if (issueMouseCommand(ENABLE_DATA_REPORT)) //Enabling mouse data reporting
    return 1;

  if (mouseIrqSet(ENABLE_IRQ_LINE)) //Enable irq line
    return 1;

  int ipc_status, r;
  message msg;
  struct packet pp; //Struct to print the mouse packet
  uint8_t counterByte = 1; //Variable that has the number of the mouse byte
  uint8_t bytes[3]; //Array that contains the 3 bytes of the mouse

  while (cnt > 0) { //Read the cnt number of packets from the mouse
    mouseByte = 0;
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) { /* received notification */
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:                             /* hardware interrupt notification */
          if (msg.m_notify.interrupts & irq_set) { /* subscribed keyboard interrupt */
            mouse_ih();
            if (counterByte == 1) {
              if ((mouseByte & BIT(3)) == 0)
                continue;
              else { //First byte received from the mouse
                bytes[0] = mouseByte;
                counterByte++; //Next byte will be the second
              }
            }
            else if (counterByte == 2) { //Second byte received from the mouse
              bytes[1] = mouseByte;
              counterByte++; //Next byte will be the third
            }
            else {
              bytes[2] = mouseByte;
              parsePacket(&pp, bytes); //Parse mouse packet
              mouse_print_packet(&pp); //Print mouse packet
              cnt--; //Decrement variable that counts mouse packets
              counterByte = 1; //Next byte will be the first
            }
          }
          break;
        default:
          break; /* no other notifications expected: do nothing */
      }
    }
  }

  if (mouseIrqSet(DISABLE_IRQ_LINE)) //Disable irq line
    return 1;

  if (issueMouseCommand(DISABLE_DATA_REPORT)) //Disable mouse data report
    return 1;

  if (mouseIrqSet(ENABLE_IRQ_LINE)) //Enable irq line
    return 1;

  if (mouse_unsubscribe_int()) { //Unsubscribing mouse interrupts
    return 1;
  }

  return 0;
}

int(mouse_test_remote)(uint16_t period, uint8_t cnt) {
  uint8_t bytes[3]; //Array that contains the 3 bytes of the mouse
  struct packet pp; //Struct to print the mouse packet
  uint8_t byte, bytesCounter = 0; //bytesCounter has the number of the mouse byte and byte has the byte from the mouse
  while (cnt > 0) { //Read the cnt number of packets from the mouse
    byte = 0;
    switch (bytesCounter) {
      case 0:
        if (issueMouseCommand(READ_DATA)) //Send command to read the mouse values
          return 1;
        byte = readMouseByte(); //Read first byte
        if ((byte & BIT(3)) == 0) { //bit(3) has to be set
          continue;
        }
        bytes[0] = byte; 
        bytesCounter = 1; //Next byte will be the second
        break;
      case 1:
        bytes[1] = readMouseByte(); //Read second byte
        bytesCounter++; //Next byte will be the third
        break;
      case 2:
        bytes[2] = readMouseByte(); //Read third byte
        bytesCounter++; //Next operation will print the packet
        break;
      case 3:
        parsePacket(&pp, bytes); //Parse mouse packet
        mouse_print_packet(&pp); //Print mouse packet
        cnt--; //Decrement variable that counts mouse packets
        bytesCounter = 0;
        tickdelay(micros_to_ticks(period * 1000)); //Wait period ms for the next byte
        break;
    }
  }

  if (issueMouseCommand(STREAM_MODE)) //Set stream mode
    return 1;

  if (issueMouseCommand(DISABLE_DATA_REPORT)) //Disable mouse data reporting
    return 1;

  if (writeDafaultCmdByte()) //Write default command byte to kbc command register
    return 1;

  return 0;
}

int(mouse_test_async)(uint8_t idle_time) {
  if (idle_time < 0) //Time is always positive
    return 1;
  else if (idle_time == 0)
    return 0; //Avoid possible problems of changing mouse configuration and time lost in the subscription and unsubscription of interrupts

  //If n is 0, the interrupts will be subscribed and unsubscribe without being handled

  //The number we use to subscribe timer interrupts and mouse interrupts has to be different
  uint8_t irq_set_timer = 2;               //This is the number we use when we subscribe the interrupt of the timer
  if (timer_subscribe_int(&irq_set_timer)) //Subscribing timer interrupts
    return 1;

  uint8_t irq_set_mouse = 4; //This is the number we use when we subscribe the interrupt of the mouse

  if (mouse_subscribe_int(&irq_set_mouse)) { //Subscribing mouse interrupts
    return 1;
  }

  if (mouseIrqSet(DISABLE_IRQ_LINE)) //Disable irq line
    return 1;

  if (issueMouseCommand(ENABLE_DATA_REPORT)) //Enabling mouse data reporting
    return 1;

  if (mouseIrqSet(ENABLE_IRQ_LINE)) //Enable irq line
    return 1;

  uint32_t freq = sys_hz(); //Get the frequency of timer 0
  int ipc_status, r;
  message msg;
  struct packet pp; //Struct to print the mouse packet
  uint8_t counterByte = 1; //Variable that has the number of the mouse byte
  uint8_t bytes[3]; //Array that contains the 3 bytes of the mouse

  counter = 0; //Timer counter = 0

  while (counter / freq < idle_time) { //Read mouse packets until there's no packet to read for idle_time secondss
    mouseByte = 0;
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) { /* received notification */
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:                                   /* hardware interrupt notification */
          if (msg.m_notify.interrupts & irq_set_mouse) { /* subscribed keyboard interrupt */
            counter = 0;
            mouse_ih(); //Handle mouse interrupt
            if ((mouseByte & BIT(3)) == 0 && counterByte == 1) { //bit(3) has to be set
              continue;
            }
            else if (counterByte == 1) { //First byte received from the mouse
              bytes[0] = mouseByte; 
              counterByte++;
            }
            else if (counterByte == 2) { //Second byte received from the mouse
              bytes[1] = mouseByte;
              counterByte++;
            }
            else {
              bytes[2] = mouseByte;
              parsePacket(&pp, bytes); //Parse mouse packet
              mouse_print_packet(&pp); //Print mouse packet
              counterByte = 1;
            }
          }
          else if (msg.m_notify.interrupts & irq_set_timer) {
            timer_int_handler(); //Handle timer interrupt
            continue;
          }
          break;
        default:
          break; /* no other notifications expected: do nothing */
      }
    }
  }

  if (mouseIrqSet(DISABLE_IRQ_LINE)) //Disable irq line
    return 1;

  if (issueMouseCommand(DISABLE_DATA_REPORT)) //Disable mouse data report
    return 1;

  if (mouseIrqSet(ENABLE_IRQ_LINE)) //Enable irq line
    return 1;

  if (mouse_unsubscribe_int()) { //Unsubscribing mouse interrupts
    return 1;
  }

  if (timer_unsubscribe_int()) //Unsubscribing timer interrupts
    return 1;

  return 0;
}

int(mouse_test_gesture)(uint8_t x_len, uint8_t tolerance) {
  uint8_t irq_set = 4; //This is the number we use when we subscribe the interrupt of the mouse

  if (mouse_subscribe_int(&irq_set)) { //Subscribing mouse interrupts
    return 1;
  }

  if (mouseIrqSet(DISABLE_IRQ_LINE)) //Disable irq line
    return 1;

  if (issueMouseCommand(ENABLE_DATA_REPORT)) //Enabling mouse data reporting
    return 1;

  if (mouseIrqSet(ENABLE_IRQ_LINE)) //Enable irq line
    return 1;

  int ipc_status, r;
  message msg;
  struct packet pp; //Struct to print the mouse packet
  uint8_t counterByte = 1, xLen = 0; //counterByte has the number of the mouse byte and xLen counts the displacement in x
  uint8_t bytes[3]; //Array that contains the 3 bytes of the mouse
  enum state_t state = INIT; //state will have a value from the enumeration state_t
  struct mouse_ev *event = &m; //Pointer to struct m that will contain an event and delta x and y
  int16_t deltaX, deltaY;

  while (state != COMP) { //Read the cnt number of packets from the mouse
    mouseByte = 0;
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) { /* received notification */
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:                             /* hardware interrupt notification */
          if (msg.m_notify.interrupts & irq_set) { /* subscribed keyboard interrupt */
            mouse_ih();
            if (counterByte == 1) {
              if ((mouseByte & BIT(3)) == 0)
                continue;
              else { //First byte received from the mouse
                bytes[0] = mouseByte;
                counterByte++;
              }
              continue;
            }
            else if (counterByte == 2) { //Second byte received from the mouse
              bytes[1] = mouseByte;
              counterByte++;
              continue;
            }
            else {
              bytes[2] = mouseByte;
              parsePacket(&pp, bytes); //Parse mouse packet
              mouse_print_packet(&pp); //Print mouse packet
              counterByte = 1;

              mouseDetectEvent(&pp, state); //event = mouse_detect_event(&pp);

              deltaX = event->delta_x;
              deltaY = event->delta_y;

              switch (state) { //State Machine
                case INIT: //Initial state
                  if (event->type == LB_PRESSED) { //If the event is LB_PRESSED, left line is started
                    xLen = 0; //Start count the displacement of x
                    state = LLINE; //State will be LLINE
                  }
                  break;
                case LLINE: //Started left line
                  xLen += abs(deltaX); //Sum of x lenght
                  if (event->type == LB_RELEASED && xLen >= x_len) { //If the event is LB_RELEASED, left line is finished
                    state = VERTEX;
                  }
                  else if (event->type != MOUSE_MOV) { //If there is other event than MOUSE_MOV, state will be INIT
                    state = INIT;
                  }
                  else {
                    if ((deltaY < 0 && abs(deltaY) > tolerance) || (deltaX < 0 && abs(deltaX) > tolerance) || abs(deltaY) < abs(deltaX)) { //If tolerance is overcomed or the slope is less than 1
                      state = INIT;
                    }
                  }
                  break;
                case VERTEX: //Reached vertex
                  if (event->type == RB_PRESSED) { //If the event is RB_PRESSED, right line is started
                    xLen = 0;//Start count the displacement of x
                    state = RLINE;//State will be RLINE
                  }
                  else if (event->type == LB_PRESSED){ //If the event is LB_PRESSED, left line is started
                    xLen = 0; //Start count the displacement of x
                    state = LLINE; //State will be LLINE
                  }
                  else if (abs(deltaX) > tolerance || abs(deltaY) > tolerance || event->type == BUTTON_EV){ //If tolerance is overcomed or another button is pressed
                    state = INIT;
                  }
                  break;
                case RLINE: //Started right line
                  xLen += abs(deltaX); //Sum of x lenght
                  if (event->type == RB_RELEASED && xLen >= x_len) { //If the event is RB_RELEASED, right line is finished
                    state = COMP; //Completed the inverted V
                  }
                  else if (event->type != MOUSE_MOV){ //If there is other event than MOUSE_MOV, state will be INIT
                    state = INIT;
                  }
                  else{
                    if ((deltaY > tolerance) || (deltaX < 0 && abs(deltaX) > tolerance) || abs(deltaY) < abs(deltaX)){ //If tolerance is overcomed or the slope is less than 1
                      state = INIT;
                    }
                  }
                  break;
                default: //Completed the inverted V
                  break;
              }
            }
          }
          break;
        default:
          break; /* no other notifications expected: do nothing */
      }
    }
  }

  if (mouseIrqSet(DISABLE_IRQ_LINE)) //Disable irq line
    return 1;

  if (issueMouseCommand(DISABLE_DATA_REPORT)) //Disable mouse data report
    return 1;

  if (mouseIrqSet(ENABLE_IRQ_LINE)) //Enable irq line
    return 1;

  if (mouse_unsubscribe_int()) { //Unsubscribing mouse interrupts
    return 1;
  }

  return 0;
}
