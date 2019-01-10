#include <lcom/lcf.h>

#include "8042.h"
#include "high_scores.h"
#include "i8254.h"
#include "keyboard.h"
#include "menu.h"
#include "mouse.h"
#include "proj.h"
#include "rtc.h"
#include "videoCard.h"
#include "xpm.h"
#include "assembly.h"

static char *background;

static enum cursor_state state;

static void get_cursor_event(struct packet *pp, bool above_box) { //Mouse cursor events: state machine
  switch (state) {
    case INITIAL: //Initial state
      if (above_box && !pp->lb)
        state = IN_BOX;
      break;
    case IN_BOX: //State where the cursor is inside the box whithout previous pressing the left button of the mouse
      if (!above_box)
        state = INITIAL;
      if (pp->lb)
        state = FINAL;
      break;
    default: //Final state: accept
      break;
  }
}

int menu() { //Menu function
  state = INITIAL;
  int isUpdating = updating_registers();

  if (isUpdating == -1)
    return -1;
  else if (isUpdating)
    tickdelay(micros_to_ticks(244));

  uint8_t hour = get_register_value(HOUR);
  uint8_t month = get_register_value(MONTH);

  if (is_BCD() == 1) {
    hour = convert_to_binary(hour);
    month = convert_to_binary(month);
  }

  uint8_t last_hour_day = get_last_hour_day(month);

  bool isDay = false;
  if (hour >= 7 && hour <= last_hour_day) //Deciding if the menu is shown in the day or night mode
    isDay = true;

  if (isDay) {
    if (vg_draw_rectangle(0, 0, get_h_res(), get_v_res(), SKY))
      return -1;
  }
  else {
    if (vg_draw_rectangle(0, 0, get_h_res(), get_v_res(), NIGHT_SKY))
      return -1;
  }

  char *second_buffer = get_second_buffer();
  memcpy(background, second_buffer, get_h_res() * get_v_res() * get_numBytes());

  //INITIALIZATION OF THE XPMS

  //Menu xpms
  Sprite *menu = create_sprite(menu_xpm, get_h_res() - 700, 20, 0, 0);
  menu->y = (int) ((double) get_v_res() / 2.0 - (double) menu->height / 2.0);

  Sprite *highscores_box = create_sprite(highscores_box_xpm, get_h_res() - 350, 0, 0, 0);
  highscores_box->y = get_v_res() / 2.0 - highscores_box->height / 2.0;

  Sprite *highscores_pressed = create_sprite(highscores_box_selected_xpm, highscores_box->x, highscores_box->y, 0, 0);

  bool hs_pres = false;

  Sprite *instructions_box = create_sprite(instructions_box_xpm, get_h_res() - 350, highscores_box->y - 20 - highscores_box->height, 0, 0);

  Sprite *instructions_pressed = create_sprite(instructions_box_selected_xpm, instructions_box->x, instructions_box->y, 0, 0);

  bool inst_pres = false;

  Sprite *play = create_sprite(play_xpm, get_h_res() - 350, instructions_box->y - 20 - instructions_box->height, 0, 0);

  Sprite *play_pressed = create_sprite(play_selected_xpm, play->x, play->y, 0, 0);

  bool play_pres = false;

  Sprite *credits_box = create_sprite(credits_box_xpm, get_h_res() - 350, 20 + highscores_box->y + highscores_box->height, 0, 0);

  Sprite *credits_pressed = create_sprite(credits_box_selected_xpm, credits_box->x, credits_box->y, 0, 0);

  bool cred_pres = false;

  Sprite *exit_box = create_sprite(exit_xpm, get_h_res() - 350, credits_box->y + credits_box->height + 20, 0, 0);

  Sprite *exit_pressed = create_sprite(exit_selected_xpm, exit_box->x, exit_box->y, 0, 0);

  bool exit_pres = false;

  if (draw_sprite(menu))
    return -1;
  if (draw_sprite(play))
    return -1;
  if (draw_sprite(instructions_box))
    return -1;
  if (draw_sprite(highscores_box))
    return -1;
  if (draw_sprite(credits_box))
    return -1;
  if (draw_sprite(exit_box))
    return -1;

  //Mouse cursors

  Sprite *cursor = create_sprite(cursor_xpm, get_h_res() / 2.0, get_v_res() / 2.0, 0, 0);

  if (draw_sprite(cursor))
    return -1;

  Sprite *hand = create_sprite(hand_xpm, cursor->x, cursor->y, 0, 0);

  //Instructions xpm
  Sprite *instructions = create_sprite(instructions_xpm, get_h_res() - 750, 20, 0, 0);
  instructions->x = (int) ((double) get_h_res() / 2.0 - (double) instructions->width / 2.0);
  instructions->y = (int) ((double) get_v_res() / 2.0 - (double) instructions->height / 2.0);

  //Credits xpm
  Sprite *credits = create_sprite(credits_xpm, get_h_res() - 700, 50, 0, 0);
  credits->x = (int) ((double) get_h_res() / 2.0 - (double) credits->width / 2.0);
  credits->y = (int) ((double) get_v_res() / 2.0 - (double) credits->height / 2.0);

  bool is_hand = false;

  copyToVRAM();

  uint8_t irq_set_kbd = 3;               //This is the number we use when we subscribe the interrupt of the keyboard
  if (kbd_subscribe_int(&irq_set_kbd)) { //Subscribing keyboard interrupts
    return -1;
  }

  uint8_t irq_set_mouse = 4;
  if (mouse_subscribe_int(&irq_set_mouse)) {
    return -1;
  }

  if (mouseIrqSet(DISABLE_IRQ_LINE)) //Disable irq line
    return -1;

  if (issueMouseCommand(ENABLE_DATA_REPORT)) //Enabling mouse data reporting
    return -1;

  if (mouseIrqSet(ENABLE_IRQ_LINE)) //Enable irq line
    return -1;

  uint8_t counterByte = 1;
  uint8_t mouse_bytes[3]; //To check if the mouse byte as the left button pressed

  int ipc_status, r;
  message msg;
  code = 0;
  mouseByte = 0;
  struct packet pp; //Struct with the mouse packet

  int option = 0;
  int previous_option = 0;

  alloc_high_scores(); //Allocating memory to high_scores_screen

  while (option == 0 || option == 2 || option == 3 || option == 4) { //The cicle ends after the time we specify when we call the function
    previous_option = option;
    code = 0;
    mouseByte = 0;
    /*You may want to use a different condition
        Get a request message.*/
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      //printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) { /* received notification */
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:                                 /* hardware interrupt notification */
          if (msg.m_notify.interrupts & irq_set_kbd) { /* subscribed keyboard interrupt */
            kbd_asm_ih();                              //Handles interrupts in assembly
            switch (option) { //4 options to menu: 0-Menu; 2-Intructions; 3-Highscores; 4-Credits
              case 0:
                if (code == ESC || code == BYTE_E) {
                  option = -1;
                }
                else if (code == BYTE_P) {
                  option = 1;
                }
                else if (code == BYTE_I) {
                  option = 2;
                }
                else if (code == BYTE_H) {
                  option = 3;
                }
                else if (code == BYTE_C) {
                  option = 4;
                }
                break;
              case 2:
              case 3:
              case 4:
                if (code == ESC || code == BYTE_E) {
                  option = 0;
                }
              default:
                break;
            }
          }
          if (msg.m_notify.interrupts & irq_set_mouse) {
            mouse_asm_ih();
            if (counterByte == 1) {
              if ((mouseByte & BIT(3)) != 0) //First byte received from the mouse
                counterByte++;               //Next byte will be the second
              mouse_bytes[0] = mouseByte;
            }
            else if (counterByte == 2) { //Second byte received from the mouse
              mouse_bytes[1] = mouseByte;
              counterByte++; //Next byte will be the third
            }
            else {
              counterByte = 1; //Next byte will be the first
              mouse_bytes[2] = mouseByte;
              parsePacket(&pp, mouse_bytes); //Parse mouse packet
                                             //New position for the cursor
              cursor->x += pp.delta_x;

              if (cursor->x > (int) get_h_res()) {
                cursor->x = get_h_res();
              }
              else if (cursor->x < 0) {
                cursor->x = 0;
              }

              cursor->y -= pp.delta_y;

              if (cursor->y > (int) get_v_res()) {
                cursor->y = get_v_res();
              }
              else if (cursor->y < 0) {
                cursor->y = 0;
              }

              memcpy(second_buffer, background, get_h_res() * get_v_res() * get_numBytes()); //Copy background to the second buffer

              switch (option) {
                case 0: {
                  //Verify if exists any collision with any box
                  if (check_cursor_collisions(cursor, play)) {
                    get_cursor_event(&pp, true);
                    if (!is_hand) {
                      is_hand = true;
                      swap_sprites(cursor, hand);
                      play_pres = true;
                      swap_sprites(play, play_pressed);
                    }
                    if (state == FINAL) {
                      state = INITIAL;
                      option = 1; //Play Game
                    }
                  }
                  else if (check_cursor_collisions(cursor, instructions_box)) {
                    get_cursor_event(&pp, true);
                    if (!is_hand) {
                      is_hand = true;
                      swap_sprites(cursor, hand);
                      inst_pres = true;
                      swap_sprites(instructions_box, instructions_pressed);
                    }
                    if (state == FINAL) {
                      state = INITIAL;
                      option = 2; //Instructions
                    }
                  }
                  else if (check_cursor_collisions(cursor, highscores_box)) {
                    get_cursor_event(&pp, true);
                    if (!is_hand) {
                      is_hand = true;
                      swap_sprites(cursor, hand);
                      hs_pres = true;
                      swap_sprites(highscores_box, highscores_pressed);
                    }
                    if (state == FINAL) {
                      state = INITIAL;
                      option = 3; //High Scores
                    }
                  }
                  else if (check_cursor_collisions(cursor, credits_box)) {
                    get_cursor_event(&pp, true);
                    if (!is_hand) {
                      is_hand = true;
                      swap_sprites(cursor, hand);
                      cred_pres = true;
                      swap_sprites(credits_box, credits_pressed);
                    }
                    if (state == FINAL) {
                      state = INITIAL;
                      option = 4; //Credits
                    }
                  }
                  else if (check_cursor_collisions(cursor, exit_box)) {
                    get_cursor_event(&pp, true);
                    if (!is_hand) {
                      is_hand = true;
                      swap_sprites(cursor, hand);
                      exit_pres = true;
                      swap_sprites(exit_box, exit_pressed);
                    }
                    if (state == FINAL) {
                      state = INITIAL;
                      option = -1;
                    }
                  }
                  else { //If there is no collision, only swap the sprites with the previous sprite was the hand
                    get_cursor_event(&pp, false);
                    if (is_hand) {
                      is_hand = false;
                      swap_sprites(cursor, hand);
                      if (play_pres) {
                        play_pres = false;
                        swap_sprites(play, play_pressed);
                      }
                      if (inst_pres) {
                        inst_pres = false;
                        swap_sprites(instructions_box, instructions_pressed);
                      }
                      if (hs_pres) {
                        hs_pres = false;
                        swap_sprites(highscores_box, highscores_pressed);
                      }
                      if (cred_pres) {
                        cred_pres = false;
                        swap_sprites(credits_box, credits_pressed);
                      }
                      if (exit_pres) {
                        exit_pres = false;
                        swap_sprites(exit_box, exit_pressed);
                      }
                    }
                  }
                  //Draw menu sprites
                  draw_sprite(menu);
                  draw_sprite(play);
                  draw_sprite(instructions_box);
                  draw_sprite(highscores_box);
                  draw_sprite(credits_box);
                  draw_sprite(exit_box);
                  draw_sprite(cursor);
                  break;
                }
                case 2:
                  if (check_cursor_collisions(cursor, exit_box)) { //Check collision with exit box
                    get_cursor_event(&pp, true);
                    if (!is_hand) {
                      is_hand = true;
                      swap_sprites(cursor, hand);
                      exit_pres = true;
                      swap_sprites(exit_box, exit_pressed);
                    }
                    if (state == FINAL) {
                      state = INITIAL;
                      option = 0; //Exit to menu
                    }
                  }
                  else { //If there is no collision, only swap the sprites with the previous sprite was the hand
                    get_cursor_event(&pp, false);
                    if (is_hand && !exit_pres) {
                      is_hand = false;
                      swap_sprites(cursor, hand);
                    }
                    else if (is_hand) {
                      is_hand = false;
                      swap_sprites(cursor, hand);
                      exit_pres = false;
                      swap_sprites(exit_box, exit_pressed);
                    }
                  }
                  //Draw Instructions Sprites
                  draw_sprite(instructions);
                  draw_sprite(exit_box);
                  draw_sprite(cursor);
                  break;
                case 3:
                  draw_high_scores(false); //Draw high_scores_screen
                  if (check_cursor_collisions(cursor, exit_box)) { //Check collision with exit box
                    get_cursor_event(&pp, true);
                    if (!is_hand) {
                      is_hand = true;
                      swap_sprites(cursor, hand);
                      exit_pres = true;
                      swap_sprites(exit_box, exit_pressed);
                    }
                    if (state == FINAL) {
                      state = INITIAL;
                      option = 0; //Exit to menu
                    }
                  }
                  else { //If there is no collision, only swap the sprites with the previous sprite was the hand
                    get_cursor_event(&pp, false);
                    if (is_hand && !exit_pres) {
                      is_hand = false;
                      swap_sprites(cursor, hand);
                    }
                    else if (is_hand) {
                      is_hand = false;
                      swap_sprites(cursor, hand);
                      exit_pres = false;
                      swap_sprites(exit_box, exit_pressed);
                    }
                  }
                  //Draw exit box and cursor
                  draw_sprite(exit_box);
                  draw_sprite(cursor);
                  break;
                case 4:
                  if (check_cursor_collisions(cursor, exit_box)) { //Check collision with exit box
                    get_cursor_event(&pp, true);
                    if (!is_hand) {
                      is_hand = true;
                      swap_sprites(cursor, hand);
                      exit_pres = true;
                      swap_sprites(exit_box, exit_pressed);
                    }
                    if (state == FINAL) {
                      state = INITIAL;
                      option = 0; //Exit to menu
                    }
                  }
                  else { //If there is no collision, only swap the sprites with the previous sprite was the hand
                    get_cursor_event(&pp, false);
                    if (is_hand && !exit_pres) {
                      is_hand = false;
                      swap_sprites(cursor, hand);
                    }
                    else if (is_hand) {
                      is_hand = false;
                      swap_sprites(cursor, hand);
                      exit_pres = false;
                      swap_sprites(exit_box, exit_pressed);
                    }
                  }
                  //Draw Credits screen
                  draw_sprite(credits);
                  draw_sprite(exit_box);
                  draw_sprite(cursor);
                  break;
                default:
                  break;
              }
              copyToVRAM(); //Copy second buffer to video memory
            }
            break;
            default:
              break; /* no other notifications expected: do nothing */
          }
      }
    }
    if (option != previous_option) { //If there was a change in menu, update boxs and cursor sprites
      if (option == 0 && (previous_option == 2 || previous_option == 3 || previous_option == 4)) {
        exit_box->y = credits_box->y + credits_box->height + 20;
        exit_box->x = credits_box->x;
      }
      else if (previous_option == 0 && (option == 2 || option == 3 || option == 4)) {
        exit_box->x = 20;
        exit_box->y = 20;
      }

      if (play_pres) {
        play_pres = false;
        swap_sprites(play, play_pressed);
      }
      if (inst_pres) {
        inst_pres = false;
        swap_sprites(instructions_box, instructions_pressed);
      }
      if (hs_pres) {
        hs_pres = false;
        swap_sprites(highscores_box, highscores_pressed);
      }
      if (cred_pres) {
        cred_pres = false;
        swap_sprites(credits_box, credits_pressed);
      }
      if (exit_pres) {
        exit_pres = false;
        swap_sprites(exit_box, exit_pressed);
      }

      memcpy(second_buffer, background, get_h_res() * get_v_res() * get_numBytes()); //Copy menu background to second buffer

      switch (option) { //Draw menu screens
        case 0:
          if (draw_sprite(menu))
            return -1;
          if (draw_sprite(play))
            return -1;
          if (draw_sprite(instructions_box))
            return -1;
          if (draw_sprite(highscores_box))
            return -1;
          if (draw_sprite(credits_box))
            return -1;
          if (draw_sprite(exit_box))
            return -1;
          if (draw_sprite(cursor))
            return -1;
          copyToVRAM();
          break;
        case 2:
          if (draw_sprite(instructions))
            return -1;
          if (draw_sprite(exit_box))
            return -1;
          if (draw_sprite(cursor))
            return -1;
          copyToVRAM();
          break;
        case 3: {
          draw_high_scores(true); //Initialize high_scores_screen pointer

          if (draw_sprite(exit_box))
            return -1;
          if (draw_sprite(cursor))
            return -1;
          copyToVRAM();
          break;
        }
        case 4:
          if (draw_sprite(credits))
            return -1;
          if (draw_sprite(exit_box))
            return -1;
          if (draw_sprite(cursor))
            return -1;
          copyToVRAM();
          break;
        default:
          break;
      }
    }
  }

  if (mouseIrqSet(DISABLE_IRQ_LINE)) //Disable irq line
    return -1;

  if (issueMouseCommand(DISABLE_DATA_REPORT)) //Disable mouse data report
    return -1;

  if (mouseIrqSet(ENABLE_IRQ_LINE)) //Enable irq line
    return -1;

  if (mouse_unsubscribe_int())
    return -1;

  if (kbd_unsubscribe_int()) { //Unsubscribing keyboard interrupts
    return -1;
  }

  //Destroy menu sprites
  destroy_sprite(menu);
  destroy_sprite(instructions);
  destroy_sprite(credits);
  destroy_sprite(play);
  destroy_sprite(instructions_box);
  destroy_sprite(highscores_box);
  destroy_sprite(credits_box);
  destroy_sprite(exit_box);
  destroy_sprite(play_pressed);
  destroy_sprite(instructions_pressed);
  destroy_sprite(credits_pressed);
  destroy_sprite(highscores_pressed);
  destroy_sprite(exit_pressed);
  destroy_sprite(cursor);
  destroy_sprite(hand);

  free_high_scores(); //Free high_scores pointer

  return option;
}

Sprite *get_character(char character) { //Get character Sprite
  switch (character) {
    case 'A':
      return create_sprite(A_xpm, 0, 0, 0, 0);
      break;
    case 'B':
      return create_sprite(B_xpm, 0, 0, 0, 0);
      break;
    case 'C':
      return create_sprite(C_xpm, 0, 0, 0, 0);
      break;
    case 'D':
      return create_sprite(D_xpm, 0, 0, 0, 0);
      break;
    case 'E':
      return create_sprite(E_xpm, 0, 0, 0, 0);
      break;
    case 'F':
      return create_sprite(F_xpm, 0, 0, 0, 0);
      break;
    case 'G':
      return create_sprite(G_xpm, 0, 0, 0, 0);
      break;
    case 'H':
      return create_sprite(H_xpm, 0, 0, 0, 0);
      break;
    case 'I':
      return create_sprite(I_xpm, 0, 0, 0, 0);
      break;
    case 'J':
      return create_sprite(J_xpm, 0, 0, 0, 0);
      break;
    case 'K':
      return create_sprite(K_xpm, 0, 0, 0, 0);
      break;
    case 'L':
      return create_sprite(L_xpm, 0, 0, 0, 0);
      break;
    case 'M':
      return create_sprite(M_xpm, 0, 0, 0, 0);
      break;
    case 'N':
      return create_sprite(N_xpm, 0, 0, 0, 0);
      break;
    case 'O':
      return create_sprite(O_xpm, 0, 0, 0, 0);
      break;
    case 'P':
      return create_sprite(P_xpm, 0, 0, 0, 0);
      break;
    case 'Q':
      return create_sprite(Q_xpm, 0, 0, 0, 0);
      break;
    case 'R':
      return create_sprite(R_xpm, 0, 0, 0, 0);
      break;
    case 'S':
      return create_sprite(S_xpm, 0, 0, 0, 0);
      break;
    case 'T':
      return create_sprite(T_xpm, 0, 0, 0, 0);
      break;
    case 'U':
      return create_sprite(U_xpm, 0, 0, 0, 0);
      break;
    case 'V':
      return create_sprite(V_xpm, 0, 0, 0, 0);
      break;
    case 'W':
      return create_sprite(W_xpm, 0, 0, 0, 0);
      break;
    case 'X':
      return create_sprite(X_xpm, 0, 0, 0, 0);
      break;
    case 'Y':
      return create_sprite(Y_xpm, 0, 0, 0, 0);
      break;
    case 'Z':
      return create_sprite(Z_xpm, 0, 0, 0, 0);
      break;
    case '0':
      return create_sprite(zero_xpm, 0, 0, 0, 0);
      break;
    case '1':
      return create_sprite(one_xpm, 0, 0, 0, 0);
      break;
    case '2':
      return create_sprite(two_xpm, 0, 0, 0, 0);
      break;
    case '3':
      return create_sprite(three_xpm, 0, 0, 0, 0);
      break;
    case '4':
      return create_sprite(four_xpm, 0, 0, 0, 0);
      break;
    case '5':
      return create_sprite(five_xpm, 0, 0, 0, 0);
      break;
    case '6':
      return create_sprite(six_xpm, 0, 0, 0, 0);
      break;
    case '7':
      return create_sprite(seven_xpm, 0, 0, 0, 0);
      break;
    case '8':
      return create_sprite(eight_xpm, 0, 0, 0, 0);
      break;
    case '9':
      return create_sprite(nine_xpm, 0, 0, 0, 0);
      break;
    default:
      return NULL;
      break;
  }
}

char *get_menu_background() { //Get Menu Background
  return background;
}

void free_menu_background() { //Free menu background memory
  free(background);
}

void menu_background_alloc() { //Allocating space for menu background
  background = malloc(get_v_res() * get_h_res() * get_numBytes());
}
