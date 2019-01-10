#include <lcom/lcf.h>

#include "lcom/pixmap.h"
#include <stdint.h>
#include <stdio.h>

// Any header files included below this line should have been created by you
#include "8042.h"
#include "i8254.h"
#include "keyboard.h"
#include "sprite.h"
#include "videoCard.h"
#include <math.h>

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");
  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab5/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab5/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(video_test_init)(uint16_t mode, uint8_t delay) {
  if (vg_init(mode) == NULL)
    return 1;

  sleep(delay);

  if (vg_exit())
    return 1;

  return 0;
}

int(video_test_rectangle)(uint16_t mode, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color) {

  if (vg_init(mode) == NULL)
    return 1;

  if (vg_draw_rectangle(x, y, width, height, color))
    return 1;


  uint8_t irq_set_kbd = 3;               //This is the number we use when we subscribe the interrupt of the keyboard
  if (kbd_subscribe_int(&irq_set_kbd)) { //Subscribing keyboard interrupts
    return 1;
  }


  int ipc_status, r;
  message msg;
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
        case HARDWARE:                                 /* hardware interrupt notification */
          if (msg.m_notify.interrupts & irq_set_kbd) { /* subscribed keyboard interrupt */
            kbc_ih();                                  //Handles interrupts in C
          }
          break;
        default:
          break; /* no other notifications expected: do nothing */
      }
    }
  }

  if (kbd_unsubscribe_int()) { //Unsubscribing keyboard interrupts
    return 1;
  }

  if (vg_exit())
    return 1;

  return 0;
}

int(video_test_pattern)(uint16_t mode, uint8_t no_rectangles, uint32_t first, uint8_t step) {

  if (vg_init(mode) == NULL)
    return 1;

  uint16_t x = 0, y = 0, width, height;
  uint32_t color;
  width = get_h_res() / no_rectangles;
  height = get_v_res() / no_rectangles;

  uint8_t bitsPerPixel = get_bits_per_pixel();
  uint8_t row = 0, col = 0;
  unsigned redScreenMask = get_redScreenMask();
  unsigned greenScreenMask = get_greenScreenMask();
  unsigned blueScreenMask = get_blueScreenMask();

  uint8_t recVertical = 0, recHorizontal = 0;

  while (recVertical < no_rectangles) {
    x = 0;
    col = 0;
    while (recHorizontal < no_rectangles) {
      if (mode == INDEXED_MODE) {
        color = (first + (row * no_rectangles + col) * step) % (1 << bitsPerPixel);
      }
      else {
        color = ((uint8_t) first + (col + row) * step) % (1 << blueScreenMask);
        color += (((uint8_t)(first >> blueScreenMask) + row * step) % (1 << (greenScreenMask)) << blueScreenMask);
        color += (((uint8_t)(first >> (blueScreenMask + greenScreenMask)) + col * step) % (1 << (redScreenMask)) << (blueScreenMask + greenScreenMask));
      }
      vg_draw_rectangle(x, y, width, height, color);
      x += width;
      col++;
      recHorizontal++;
    }
    y += height;
    row++;
    recHorizontal = 0;
    recVertical++;
  }

  uint8_t irq_set_kbd = 3;               //This is the number we use when we subscribe the interrupt of the keyboard
  if (kbd_subscribe_int(&irq_set_kbd)) { //Subscribing keyboard interrupts
    return 1;
  }
  int ipc_status, r;
  message msg;
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
        case HARDWARE:                                 /* hardware interrupt notification */
          if (msg.m_notify.interrupts & irq_set_kbd) { /* subscribed keyboard interrupt */
            kbc_ih();                                  //Handles interrupts in C
          }

          break;
        default:
          break; /* no other notifications expected: do nothing */
      }
    }
  }

  if (kbd_unsubscribe_int()) { //Unsubscribing keyboard interrupts
    return 1;
  }

  if (vg_exit())
    return 1;

  return 0;
}

int(video_test_xpm)(const char *xpm[], uint16_t x, uint16_t y) {

  if (vg_init(INDEXED_MODE) == NULL)
    return 1;

  Sprite *s = create_sprite(xpm, x, y, 0, 0);
  if (s == NULL)
    return 1;

  for (uint8_t i = 0; i < s->height; i++, y++) {
    for (uint8_t j = 0; j < s->width; j++) {
      drawPixel(x, y, *(s->map + i * s->width + j));
      x++;
    }
    x -= s->width;
  }

  uint8_t irq_set_kbd = 3;               //This is the number we use when we subscribe the interrupt of the keyboard
  if (kbd_subscribe_int(&irq_set_kbd)) { //Subscribing keyboard interrupts
    return 1;
  }

  int ipc_status, r;
  message msg;
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
        case HARDWARE:                                 /* hardware interrupt notification */
          if (msg.m_notify.interrupts & irq_set_kbd) { /* subscribed keyboard interrupt */
            kbc_ih();                                  //Handles interrupts in C
          }
          break;
        default:
          break; /* no other notifications expected: do nothing */
      }
    }
  }

  if (kbd_unsubscribe_int()) { //Unsubscribing keyboard interrupts
    return 1;
  }

  destroy_sprite(s);

  if (vg_exit())
    return 1;

  return 0;
}

int(video_test_move)(const char *xpm[], uint16_t xi, uint16_t yi,
                     uint16_t xf, uint16_t yf, int16_t speed, uint8_t fr_rate) {

  if (vg_init(INDEXED_MODE) == NULL)
    return 1;

  Sprite *s = create_sprite(xpm, xi, yi, 0, 0);
  if (s == NULL)
    return 1;

  if (timer_set_frequency(0, fr_rate))
    return 1;

  //The number we use to subscribe timer interrupts and mouse interrupts has to be different
  uint8_t irq_set_timer = 2;               //This is the number we use when we subscribe the interrupt of the timer
  if (timer_subscribe_int(&irq_set_timer)) //Subscribing timer interrupts
    return 1;

  uint8_t irq_set_kbd = 3;               //This is the number we use when we subscribe the interrupt of the keyboard
  if (kbd_subscribe_int(&irq_set_kbd)) { //Subscribing keyboard interrupts
    return 1;
  }

  char *video_mem = get_video_mem();
  unsigned h_res = get_h_res(), v_res = get_v_res();

  int ipc_status, r;
  message msg;
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
        case HARDWARE:                                 /* hardware interrupt notification */
          if (msg.m_notify.interrupts & irq_set_kbd) { /* subscribed keyboard interrupt */
            kbc_ih();                                  //Handles interrupts in C
          }
          else if (msg.m_notify.interrupts & irq_set_timer) {
            if (((xi % h_res) == xf && (yi % v_res) == yf) || xi > h_res || yi > v_res)
              continue;

            if (speed < 0) {
              timer_int_handler(); //Handle timer interrupt
              if (counter == (unsigned) abs(speed)) {
                memset(video_mem, 0, get_numBytes() * h_res * v_res);
                if (xi == xf) {
                  yi += abs(speed);
                }
                else { //yi == yf
                  xi += abs(speed);
                }
                s->x = xi;
                s->y = yi;

                for (uint8_t i = 0; i < s->height; i++, s->y++) {
                  for (uint8_t j = 0; j < s->width; j++) {
                    drawPixel(s->x % h_res, s->y % v_res, *(s->map + i * s->width + j));
                    s->x++;
                  }
                  s->x -= s->width;
                }
                counter = 0;
              }
            }
            else {
              memset(video_mem, 0, get_numBytes() * h_res * v_res);
              if (xi == xf) {
                yi += speed;
              }
              else { //yi == yf
                xi += speed;
              }

              s->x = xi;
              s->y = yi;

              for (uint8_t i = 0; i < s->height; i++, s->y++) {
                for (uint8_t j = 0; j < s->width; j++) {
                  drawPixel(s->x % h_res, s->y % v_res, *(s->map + i * s->width + j));
                  s->x++;
                }
                s->x -= s->width;
              }
            }
          }
          break;
        default:
          break; /* no other notifications expected: do nothing */
      }
    }
  }

  if (kbd_unsubscribe_int()) { //Unsubscribing keyboard interrupts
    return 1;
  }

  if (timer_unsubscribe_int()) //Unsubscribing timer interrupts
    return 1;

  destroy_sprite(s);

  if (vg_exit())
    return 1;

  return 0;
}

//Util functions for tmer_set_frequency()
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

int(video_test_controller)() {
  vg_vbe_contr_info_t info_p;
  VbeInfoBlock * v = NULL;
  char *firstPos = lm_init(false);
  if ((v = getControllerInfo()) == NULL)
    return 1;

  info_p.VBESignature[0] = v->VbeSignature[0];
  info_p.VBESignature[1] = v->VbeSignature[1];
  info_p.VBESignature[2] = v->VbeSignature[2];
  info_p.VBESignature[3] = v->VbeSignature[3];
  info_p.VBEVersion[0] = (uint8_t)v->VbeVersion+1;
  info_p.VBEVersion[1] = (uint8_t)v->VbeVersion+0;
  info_p.OEMString = ((v->OemStringPtr>>16)<<4)+(v->OemStringPtr & 0xFFFF) + firstPos;
  
  info_p.VideoModeList = ((v->VideoModePtr>>16)<<4)+(v->VideoModePtr & 0xFFFF) + (uint16_t*)firstPos;
  info_p.TotalMemory = v->TotalMemory;
  info_p.OEMVendorNamePtr = ((v->OemVendorNamePtr>>16)<<4)+(v->OemVendorNamePtr & 0xFFFF) + firstPos;
  info_p.OEMProductNamePtr = ((v->OemProductNamePtr>>16)<<4)+(v->OemProductNamePtr & 0xFFFF) + firstPos;
  info_p.OEMProductRevPtr = ((v->OemProductRevPtr>>16)<<4)+(v->OemProductRevPtr & 0xFFFF) + firstPos;
  
  vg_display_vbe_contr_info(&info_p);

  return 0;
}
