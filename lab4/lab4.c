#include <lcom/lcf.h>
#include <lcom/timer.h>

#include <stdint.h>
#include <stdio.h>

// Any header files included below this line should have been created by you
#include "8042.h"
#include "i8254.h"
#include "mouse.h"

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

  if (mouse_enable_data_reporting())
    return 1;

  if (mouse_subscribe_int(&irq_set)) { //Subscribing mouse interrupts
    return 1;
  }

  int ipc_status, r;
  message msg;
  struct packet pp;
  uint8_t counterByte = 1;
  uint8_t bytes[3];

  while (cnt > 0) { //Read the cnt number of packets from the mouse
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) { /* received notification */
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:                             /* hardware interrupt notification */
          if (msg.m_notify.interrupts & irq_set) { /* subscribed keyboard interrupt */
            mouse_ih();
            if ((mouseByte & BIT(3)) == 0 && counterByte == 1) {
              continue;
            }
            else if (counterByte == 1) { //First byte received from the mouse
              bytes[0] = mouseByte;
              counterByte++;
              continue;
            }
            else if (counterByte == 2) { //Second byte received from the mouse
              bytes[1] = mouseByte;
              counterByte++;
              continue;
            }
            else {
              bytes[2] = mouseByte;
            }
          }
          break;
        default:
          break; /* no other notifications expected: do nothing */
      }
    }
    pp.bytes[0] = bytes[0];
    pp.bytes[1] = bytes[1];
    pp.bytes[2] = bytes[2];
    pp.rb = ((bytes[0] & RB) >> 1);
    pp.mb = ((bytes[0] & MB) >> 2);
    pp.lb = (bytes[0] & LB);
    pp.delta_x = bytes[1];
    pp.delta_y = bytes[2];
    pp.x_ov = ((bytes[0] & X_OV) >> 6);
    pp.y_ov = ((bytes[0] & Y_OV) >> 7);

    mouse_print_packet(&pp);
    cnt--;
    counterByte = 1;
  }

  if (mouse_unsubscribe_int()) { //Unsubscribing mouse interrupts
    return 1;
  }

  if (disableDataReport()) //Disable mouse data report
    return 1;

  return 0;
}

int(mouse_test_remote)(uint16_t period, uint8_t cnt) {
  if (setRemoteMode())
    return 1;

  //uint8_t numPack = cnt;
  uint8_t bytes[3];
  struct packet pp;

  while (cnt > 0) {
    if (issueReadData())
      return 1;

    mouse_ih();

    if ((mouseByte & BIT(3)) == 0) {
      continue;
    }
    else {
      bytes[0] = mouseByte;
      bytes[1] = readMouseByte();
      bytes[2] = readMouseByte();
    }

    pp.bytes[0] = bytes[0];
    pp.bytes[1] = bytes[1];
    pp.bytes[2] = bytes[2];
    pp.rb = ((bytes[0] & RB) >> 1);
    pp.mb = ((bytes[0] & MB) >> 2);
    pp.lb = (bytes[0] & LB);
    pp.delta_x = bytes[1];
    pp.delta_y = bytes[2];
    pp.x_ov = ((bytes[0] & X_OV) >> 6);
    pp.y_ov = ((bytes[0] & Y_OV) >> 7);
    mouse_print_packet(&pp);
    tickdelay(micros_to_ticks(period));
    cnt--;
  }

  if (setStreamMode())
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

  if (mouse_enable_data_reporting())
    return 1;

  if (mouse_subscribe_int(&irq_set_mouse)) { //Subscribing mouse interrupts
    return 1;
  }
  uint32_t freq = sys_hz();
  int ipc_status, r;
  message msg;
  struct packet pp;
  uint8_t counterByte = 1;
  uint8_t bytes[3];

  counter = 0;

  while (counter/freq < idle_time) { 
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) { /* received notification */
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:                             /* hardware interrupt notification */
          if (msg.m_notify.interrupts & irq_set_mouse) { /* subscribed keyboard interrupt */
            mouse_ih();
            if ((mouseByte & BIT(3)) == 0 && counterByte == 1) {
              continue;
            }
            else if (counterByte == 1) { //First byte received from the mouse
              bytes[0] = mouseByte;
              counterByte++;
              continue;
            }
            else if (counterByte == 2) { //Second byte received from the mouse
              bytes[1] = mouseByte;
              counterByte++;
              continue;
            }
            else {
              bytes[2] = mouseByte;
            }
          }
          else if (msg.m_notify.interrupts & irq_set_timer){
            timer_int_handler();
            continue;
          }
          break;
        default:
          break; /* no other notifications expected: do nothing */
      }
    }
    pp.bytes[0] = bytes[0];
    pp.bytes[1] = bytes[1];
    pp.bytes[2] = bytes[2];
    pp.rb = ((bytes[0] & RB) >> 1);
    pp.mb = ((bytes[0] & MB) >> 2);
    pp.lb = (bytes[0] & LB);
    pp.delta_x = bytes[1];
    pp.delta_y = bytes[2];
    pp.x_ov = ((bytes[0] & X_OV) >> 6);
    pp.y_ov = ((bytes[0] & Y_OV) >> 7);

    mouse_print_packet(&pp);
    counterByte = 1;
    counter = 0;
  }

  if (timer_unsubscribe_int()) //Unsubscribing timer interrupts
    return 1;

  if (mouse_unsubscribe_int()) { //Unsubscribing mouse interrupts
    return 1;
  }

  if (disableDataReport()) //Disable mouse data report
    return 1;

  return 0;
}

int(mouse_test_gesture)() {
  /* To be completed */
  printf("%s: under construction\n", __func__);
  return 1;
}
