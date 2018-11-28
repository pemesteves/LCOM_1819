#include "lcom/pixmap.h"
#include <lcom/lcf.h>
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

  uint8_t irq_set_timer = 2; //This is the number we use when we subscribe the interrupt
  if (timer_subscribe_int(&irq_set_timer))
    return 1;

  int ipc_status, r;
  uint32_t freq = sys_hz();
  message msg;
  while (delay > counter / freq) { //The cicle ends after the time we specify when we call the function
    /*You may want to use a different condition
        Get a request message.*/
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) { /* received notification */
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:                                   /* hardware interrupt notification */
          if (msg.m_notify.interrupts & irq_set_timer) { /* subscribed interrupt */

            timer_int_handler(); //Counting the number of interruptions
          }
          break;
        default:
          break; /* no other notifications expected: do nothing */
      }
    }
  }

  if (timer_unsubscribe_int())
    return 1;

  if (vg_exit())
    return 1;

  return 0;
}

int(video_test_rectangle)(uint16_t mode, uint16_t x, uint16_t y,
                          uint16_t width, uint16_t height, uint32_t color) {

  if (vg_init(mode) == NULL)
    return 1;

  /*if (x + width > get_h_res() || y + height > get_v_res()) {
    vg_exit();
    return 1;
  }*/

  /*
  while (height > 0) {
    if (vg_draw_hline(x, y, width, color))
      return 1;
    y++;
    height--;
  }*/
  vg_draw_rectangle(x, y, width, height, color);

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

  uint8_t UNUSED(numBytes) = get_numBytes(), bitsPerPixel = get_bits_per_pixel();
  uint8_t row = 0, col = 0;
  unsigned redScreenMask = get_redScreenMask();
  unsigned greenScreenMask = get_greenScreenMask();
  unsigned blueScreenMask = get_blueScreenMask();
  /*
R(row, col) = (R(first) + col * step) % (1 << RedScreeMask) 
G(row, col) = (G(first) + row * step) % (1 << GreenScreeMask) 
B(row, col) = (B(first) + (col + row) * step) % (1 << BlueScreeMask) 
*/
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
    kbd_print_no_sysinb(counterSysIn);
    return 1;
  }

  if (vg_exit())
    return 1;

  return 0;
}

int(video_test_xpm)(const char *xpm[], uint16_t x, uint16_t y) {

  if (vg_init(INDEXED_MODE) == NULL)
    return 1;

  //int width, height;
  //char *map;

  Sprite *s = create_sprite(xpm, x, y, 0, 0);
  if (s == NULL)
    return 1;

  char *video_mem = get_video_mem();
  unsigned numBytes = get_numBytes();
  video_mem += (y * get_h_res() + x) * numBytes;


  for (uint8_t i = 0; i < s->height; i++, y++) {
    for (uint8_t j = 0; j < s->width; j++) {
      /*for (uint8_t i = 0; i < numBytes; i++) {
        tmp_color = s->map[counter] >> 8 * i;
        *video_mem = tmp_color;
        video_mem++;
      }*/
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
                     uint16_t UNUSED(xf), uint16_t UNUSED(yf), int16_t speed, uint8_t UNUSED(fr_rate)) {

  if (vg_init(INDEXED_MODE) == NULL)
    return 1;

  Sprite * s = create_sprite(xpm, xi, yi,speed,speed);
  if (s == NULL)
    return 1;

  char *video_mem = get_video_mem();
  unsigned numBytes = get_numBytes();
  video_mem += (yi * get_h_res() + xi) * numBytes;

  uint8_t irq_set_timer = 2; //This is the number we use when we subscribe the interrupt
  if (timer_subscribe_int(&irq_set_timer))
    return 1;

  uint8_t irq_set_kbd = 3;               //This is the number we use when we subscribe the interrupt of the keyboard
  if (kbd_subscribe_int(&irq_set_kbd)) { //Subscribing keyboard interrupts
    return 1;
  }

  int ipc_status, r;
  message msg;
  while (code != ESC) { //The cicle ends after
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
          if (msg.m_notify.interrupts & irq_set_timer) { /* subscribed interrupt */
            timer_int_handler(); //Counting the number of interruptions
            /*if (xi==xf){ //Vertical direction
              if (speed > 0){ //Displacement in pixels between consecutive frames
      
              }
              else if(speed < 0){ //Number of frames required for a displacement of one pixel

              }
            }
            else if(yi==yf){ //Horizontal direction
              if (speed > 0){ //Displacement in pixels between consecutive frames

              }
              else if(speed < 0){ //Number of frames required for a displacement of one pixel
      
              }
            }*/
            char *video_mem = get_video_mem();
            unsigned numBytes = get_numBytes();
            video_mem += (yi * get_h_res() + xi) * numBytes;
          }
          else if (msg.m_notify.interrupts & irq_set_kbd) { /* subscribed keyboard interrupt */
            kbc_ih();
          }
          break;
        default:
          break; /* no other notifications expected: do nothing */
      }
    }
  }

  if(timer_unsubscribe_int()){
    return 1;
  }
  
  if (kbd_unsubscribe_int()) { //Unsubscribing keyboard interrupts
    return 1;
  }

  destroy_sprite(s);

  if (vg_exit())
    return 1;

  return 0;
}

int(video_test_controller)() {
  /*vg_vbe_contr_info_t contr_info;
  vbe_mode_info_t mode_info;

  if(vbeGetModeInfo(INDEXED_MODE, &mode_info))
    return 1;

  vg_display_vbe_contr_info(&contr_info);
  */

  return 0;
}
