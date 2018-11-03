#include <lcom/lcf.h>

#include <lcom/lab2.h>
#include <lcom/timer.h>

#include "i8254.h"
#include <stdbool.h>
#include <stdint.h>

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need/ it]
  lcf_trace_calls("/home/lcom/labs/lab2/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab2/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(timer_test_read_config)(uint8_t timer, enum timer_status_field field) {

  uint8_t st; //This variable will have the Read-Back Status
  if (timer_get_conf(timer, &st))
    return 1;
  if (timer_display_conf(timer, st, field))
    return 1;

  return 0;
}

int(timer_test_time_base)(uint8_t timer, uint32_t freq) {
  //This function only calls timer_set_frequency()
  if (timer_set_frequency(timer, freq))
    return 1;

  return 0;
}

int(timer_test_int)(uint8_t time) {

  uint8_t irq_set = 2; //This is the number we use when we subscribe the interrupt
  if (timer_subscribe_int(&irq_set))
    return 1;

  int ipc_status, r;
  message msg;
  while (time > counter / 60) { //The cicle ends after the time we specify when we call the function
    /*You may want to use a different condition
        Get a request message.*/
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) { /* received notification */
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:                             /* hardware interrupt notification */
          if (msg.m_notify.interrupts & irq_set) { /* subscribed interrupt */

            timer_int_handler(); //Counting the number of interruptions

            if (counter % 60 == 0) { //Print a message every second
              timer_print_elapsed_time();
            }
          }
          break;
        default:
          break; /* no other notifications expected: do nothing */
      }
    }
  }

  if (timer_unsubscribe_int())
    return 1;

  return 0;
}

int(util_get_LSB)(uint16_t val, uint8_t *lsb) {

  uint8_t byte = LARGE_BIN_NUMBER; //This is the largest number we can represent without signal
  byte &= val;                     //Now byte should have the result from a bitwise and with the LSB of val
  *lsb = byte;
  return 0;
}

int(util_get_MSB)(uint16_t val, uint8_t *msb) {

  uint8_t byte = LARGE_BIN_NUMBER; //This is the largest number we can represent without signal
  byte &= (val >> 8);              //Now byte should have the result from a bitwise and with the MSB of val
  *msb = byte;
  return 0;
}
