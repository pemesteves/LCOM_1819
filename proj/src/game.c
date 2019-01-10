#include <lcom/lcf.h>
// Any header files included below this line should have been created by you
#include "8042.h"
#include "animSprite.h"
#include "assembly.h"
#include "game.h"
#include "high_scores.h"
#include "i8254.h"
#include "keyboard.h"
#include "menu.h"
#include "mouse.h"
#include "proj.h"
#include "rtc.h"
#include "videoCard.h"
#include "xpm.h"

static char *background;

int game(uint16_t xi, uint16_t yi, uint8_t fr_rate) { //Game Function

  int isUpdating = updating_registers();

  if (isUpdating == -1)
    return 1;
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
  if (hour >= 7 && hour <= last_hour_day) //Decides if the player plays the day or night mode
    isDay = true;

  srand(time(NULL)); //Initialize rand() function

  if (isDay) { //Draw background for the first time
    if (vg_draw_rectangle(0, 0, get_h_res(), get_v_res(), SKY))
      return -1;
  }
  else {
    if (vg_draw_rectangle(0, 0, get_h_res(), get_v_res(), NIGHT_SKY))
      return -1;
  }

  Sprite *scorexpm = create_sprite(score_xpm, 10, 20, 0, 0);

  Sprite *star;

  if (isDay) { //Draw the star for the first time
    star = create_sprite(sun_xpm, get_h_res() - 120, 20, 0, 0);
  }
  else {
    star = create_sprite(moon_xpm, get_h_res() - 130, 20, 0, 0);
  }

  //Initialize all the Sprites and AnimSprites
  Sprite *initial = create_sprite(start_xpm, xi, yi, 0, 0);
  if (initial == NULL)
    return 1;

  Sprite *jump = create_sprite(jump_xpm, xi, yi, 0, -32);
  if (jump == NULL)
    return 1;

  Sprite *other_jump = create_sprite(no_tongue_jump_xpm, xi, yi, 0, -32);
  if (other_jump == NULL)
    return 1;

  AnimSprite *rocky_animation = create_animSprite(xi, yi, 4, start_xpm, jump_xpm, run_xpm, jump_xpm);
  if (rocky_animation == NULL)
    return 1;

  AnimSprite *other_rocky_animation = create_animSprite(xi, yi, 4, no_tongue_start_xpm, no_tongue_jump_xpm, no_tongue_run_xpm, no_tongue_jump_xpm);
  if (other_rocky_animation == NULL)
    return 1;

  Sprite *arrow = create_sprite(arrow_xpm, 205, 192 + star->height, 0, 0);
  if (arrow == NULL)
    return 1;

  //Enemies
  Sprite *bush = create_sprite(bush_xpm, get_h_res(), yi + 10, -7, 0);
  if (bush == NULL)
    return 1;

  Sprite *spike_monster = create_sprite(spike_monster_xpm, get_h_res(), yi + 13, -7, 0);
  if (spike_monster == NULL)
    return 1;

  Sprite *spike = create_sprite(spike_xpm, get_h_res(), yi + 24, -7, 0);
  if (spike == NULL)
    return 1;

  Sprite *dragon = create_sprite(dragon_xpm, get_h_res(), star->y + star->height + 100, -7, 0);
  if (dragon == NULL)
    return 1;

  Sprite *bird = create_sprite(bird_xpm, get_h_res(), star->y + star->height + 100, -7, 0);
  if (bird == NULL)
    return 1;

  Sprite *enemies[] = {bush, spike_monster, spike, dragon, bird};
  Sprite *enemies_on_the_screen[5] = {NULL, NULL, NULL, NULL, NULL};

  //Asking for the player name before the game start

  Sprite *name = create_sprite(name_xpm, get_h_res() - 700, 50, 0, 0);
  name->x = (int) ((double) get_h_res() / 2.0 - (double) name->width / 2.0);
  name->y = (int) ((double) get_v_res() / 2.0 - (double) name->height / 2.0);
  Sprite *letter = NULL;
  int x = name->x + 180;
  int y = name->y + 145;
  char player_name[13] = {' '};
  int8_t num_letters = 0;

  bool game_not_started = true;
  bool first_time_drawing_background = true;

  if (draw_sprite(name))
    return -1;

  copyToVRAM(); //Copy second buffer to video memory

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

  uint8_t irq_set_mouse = 4;
  if (mouse_subscribe_int(&irq_set_mouse)) {
    return 1;
  }

  if (mouseIrqSet(DISABLE_IRQ_LINE)) //Disable irq line
    return 1;

  if (issueMouseCommand(ENABLE_DATA_REPORT)) //Enabling mouse data reporting
    return 1;

  if (mouseIrqSet(ENABLE_IRQ_LINE)) //Enable irq line
    return 1;

  //char *video_mem = get_video_mem();
  uint8_t counterByte = 1;
  uint8_t firstMouseByte; //To check ig the mouse byte as the left button pressed

  uint32_t score = 0;
  uint32_t high_scores[10] = {0}; //Array with the high scores

  bool startedJump = false;

  uint32_t time_to_create_enemy = 80;

  Sprite *dead;
  bool rockyDead = false;
  bool noTongue = false;

  bool finish_game = false;

  int ipc_status, r;
  message msg;
  code = 0;
  mouseByte = 0;
  while (!finish_game) { //The cicle ends after the time we specify when we call the function
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
        case HARDWARE:                                   /* hardware interrupt notification */
          if ((msg.m_notify.interrupts & irq_set_kbd)) { /* subscribed keyboard interrupt */
            kbd_asm_ih();                                //Handles interrupts in assembly

            if (game_not_started) { //If the game is not started, the player have to give his name
              switch (code) {       //The keyboard scancode corresponds to a letter or space or backspace or enter: other scancodes doesn't influence this switch...case
                case BYTE_A:
                  if (num_letters < 13) {
                    letter = create_sprite(A_xpm, x, y, 0, 0);
                    x += 30;
                    player_name[num_letters] = 'A';
                    num_letters++;
                  }
                  break;
                case BYTE_B:
                  if (num_letters < 13) {
                    letter = create_sprite(B_xpm, x, y, 0, 0);
                    x += 30;
                    player_name[num_letters] = 'B';
                    num_letters++;
                  }
                  break;
                case BYTE_C:
                  if (num_letters < 13) {
                    letter = create_sprite(C_xpm, x, y, 0, 0);
                    x += 30;
                    player_name[num_letters] = 'C';
                    num_letters++;
                  }
                  break;
                case BYTE_D:
                  if (num_letters < 13) {
                    letter = create_sprite(D_xpm, x, y, 0, 0);
                    x += 30;
                    player_name[num_letters] = 'D';
                    num_letters++;
                  }
                  break;
                case BYTE_E:
                  if (num_letters < 13) {
                    letter = create_sprite(E_xpm, x, y, 0, 0);
                    x += 30;
                    player_name[num_letters] = 'E';
                    num_letters++;
                  }
                  break;
                case BYTE_F:
                  if (num_letters < 13) {
                    letter = create_sprite(F_xpm, x, y, 0, 0);
                    x += 30;
                    player_name[num_letters] = 'F';
                    num_letters++;
                  }
                  break;
                case BYTE_G:
                  if (num_letters < 13) {
                    letter = create_sprite(G_xpm, x, y, 0, 0);
                    x += 30;
                    player_name[num_letters] = 'G';
                    num_letters++;
                  }
                  break;
                case BYTE_H:
                  if (num_letters < 13) {
                    letter = create_sprite(H_xpm, x, y, 0, 0);
                    x += 30;
                    player_name[num_letters] = 'H';
                    num_letters++;
                  }
                  break;
                case BYTE_I:
                  if (num_letters < 13) {
                    letter = create_sprite(I_xpm, x, y, 0, 0);
                    x += 30;
                    player_name[num_letters] = 'I';
                    num_letters++;
                  }
                  break;
                case BYTE_J:
                  if (num_letters < 13) {
                    letter = create_sprite(J_xpm, x, y, 0, 0);
                    x += 30;
                    player_name[num_letters] = 'J';
                    num_letters++;
                  }
                  break;
                case BYTE_K:
                  if (num_letters < 13) {
                    letter = create_sprite(K_xpm, x, y, 0, 0);
                    x += 30;
                    player_name[num_letters] = 'K';
                    num_letters++;
                  }
                  break;
                case BYTE_L:
                  if (num_letters < 13) {
                    letter = create_sprite(L_xpm, x, y, 0, 0);
                    x += 30;
                    player_name[num_letters] = 'L';
                    num_letters++;
                  }
                  break;
                case BYTE_M:
                  if (num_letters < 13) {
                    letter = create_sprite(M_xpm, x, y, 0, 0);
                    x += 30;
                    player_name[num_letters] = 'M';
                    num_letters++;
                  }
                  break;
                case BYTE_N:
                  if (num_letters < 13) {
                    letter = create_sprite(N_xpm, x, y, 0, 0);
                    x += 30;
                    player_name[num_letters] = 'N';
                    num_letters++;
                  }
                  break;
                case BYTE_O:
                  if (num_letters < 13) {
                    letter = create_sprite(O_xpm, x, y, 0, 0);
                    x += 30;
                    player_name[num_letters] = 'O';
                    num_letters++;
                  }
                  break;
                case BYTE_P:
                  if (num_letters < 13) {
                    letter = create_sprite(P_xpm, x, y, 0, 0);
                    x += 30;
                    player_name[num_letters] = 'P';
                    num_letters++;
                  }
                  break;
                case BYTE_Q:
                  if (num_letters < 13) {
                    letter = create_sprite(Q_xpm, x, y, 0, 0);
                    x += 30;
                    player_name[num_letters] = 'Q';
                    num_letters++;
                  }
                  break;
                case BYTE_R:
                  if (num_letters < 13) {
                    letter = create_sprite(R_xpm, x, y, 0, 0);
                    x += 30;
                    player_name[num_letters] = 'R';
                    num_letters++;
                  }
                  break;
                case BYTE_S:
                  if (num_letters < 13) {
                    letter = create_sprite(S_xpm, x, y, 0, 0);
                    x += 30;
                    player_name[num_letters] = 'S';
                    num_letters++;
                  }
                  break;
                case BYTE_T:
                  if (num_letters < 13) {
                    letter = create_sprite(T_xpm, x, y, 0, 0);
                    x += 30;
                    player_name[num_letters] = 'T';
                    num_letters++;
                  }
                  break;
                case BYTE_U:
                  if (num_letters < 13) {
                    letter = create_sprite(U_xpm, x, y, 0, 0);
                    x += 30;
                    player_name[num_letters] = 'U';
                    num_letters++;
                  }
                  break;
                case BYTE_V:
                  if (num_letters < 13) {
                    letter = create_sprite(V_xpm, x, y, 0, 0);
                    x += 30;
                    player_name[num_letters] = 'V';
                    num_letters++;
                  }
                  break;
                case BYTE_W:
                  if (num_letters < 13) {
                    letter = create_sprite(W_xpm, x, y, 0, 0);
                    x += 30;
                    player_name[num_letters] = 'W';
                    num_letters++;
                  }
                  break;
                case BYTE_X:
                  if (num_letters < 13) {
                    letter = create_sprite(X_xpm, x, y, 0, 0);
                    x += 30;
                    player_name[num_letters] = 'X';
                    num_letters++;
                  }
                  break;
                case BYTE_Y:
                  if (num_letters < 13) {
                    letter = create_sprite(Y_xpm, x, y, 0, 0);
                    x += 30;
                    player_name[num_letters] = 'Y';
                    num_letters++;
                  }
                  break;
                case BYTE_Z:
                  if (num_letters < 13) {
                    letter = create_sprite(Z_xpm, x, y, 0, 0);
                    x += 30;
                    player_name[num_letters] = 'Z';
                    num_letters++;
                  }
                  break;
                case BYTE_SPACE: //The space will be transformed in a underscore in the files
                  if (num_letters < 13) {
                    x += 30;
                    player_name[num_letters] = '_';
                    num_letters++;
                    continue;
                  }
                  break;
                case BACKSPACE: //The backspace deletes a character form the screen and the player name
                  x -= 30;
                  player_name[num_letters] = ' ';
                  num_letters--;

                  if (x < name->x + 180)
                    x = name->x + 180;

                  if (num_letters < 0)
                    num_letters = 0;

                  if (isDay) {
                    if (vg_draw_rectangle(x, y, 30, 30, SKY))
                      return -1;
                  }
                  else {
                    if (vg_draw_rectangle(x, y, 30, 30, NIGHT_SKY))
                      return -1;
                  }

                  copyToVRAM();
                  continue;
                  break;
                case ENTER:              //Enter makes the name be accepted
                  if (num_letters > 0) { //The name can't be empty
                    clear_game_screen(isDay, first_time_drawing_background);
                    game_not_started = false;
                    first_time_drawing_background = false;
                  }
                  else
                    continue;
                  break;
                default: //Do nothing
                  continue;
                  break;
              }
              draw_sprite(letter);
              copyToVRAM();
              destroy_sprite(letter);
            }
            else {
              if (code == ESC) { //Escape from the game
                finish_game = true;
              }

              if (code == BYTE_T) { //T: puts the character tongue inside his mouth
                noTongue = !noTongue;
                swap_animSprites(rocky_animation, other_rocky_animation);
                rocky_animation->sp = other_rocky_animation->sp;
                swap_sprites(jump, other_jump);
              }

              if (!startedJump) {
                if ((code == UP_ARROW || code == BYTE_SPACE || code == BYTE_W) && !rockyDead) {
                  startedJump = true;
                }
              }

              if (rockyDead) {                               //Dead screen: select continue game or exit to menu
                if (code == RIGHT_ARROW || code == BYTE_D) { //right arrow or D
                  if (arrow->x == 205) {
                    if (isDay) {
                      if (vg_draw_rectangle(arrow->x, arrow->y, arrow->width, arrow->height, SKY))
                        return 1;
                    }
                    else {
                      if (vg_draw_rectangle(arrow->x, arrow->y, arrow->width, arrow->height, NIGHT_SKY))
                        return 1;
                    }
                    arrow->x = 375;
                    draw_sprite(arrow);
                    copyToVRAM();
                  }
                }
                else if (code == LEFT_ARROW || code == BYTE_A) { //left arrow or A
                  if (arrow->x == 375) {
                    if (isDay) {
                      if (vg_draw_rectangle(arrow->x, arrow->y, arrow->width, arrow->height, SKY))
                        return 1;
                    }
                    else {
                      if (vg_draw_rectangle(arrow->x, arrow->y, arrow->width, arrow->height, NIGHT_SKY))
                        return 1;
                    }
                    arrow->x = 205;
                    draw_sprite(arrow);
                    copyToVRAM();
                  }
                }
                else if (code == ENTER) { //Enter: select option
                  if (arrow->x == 205) {
                    rockyDead = !rockyDead;
                    score = 0;

                    counter = 0;
                    bush->xspeed = -7;
                    spike_monster->xspeed = -7;
                    spike->xspeed = -7;
                    dragon->xspeed = -7;
                    bird->xspeed = -7;

                    time_to_create_enemy = 80;

                    jump->y = yi;
                    jump->yspeed = -32;
                    reinitialize_jumpPosition();
                    startedJump = false;

                    while (rocky_animation->cur_fig != 0) {
                      animate_animSprite(rocky_animation);
                    }
                    rocky_animation->cur_aspeed = 0;
                    destroy_enemies(sizeof(enemies_on_the_screen) / sizeof(Sprite *), enemies_on_the_screen);
                  }
                  else {
                    finish_game = true;
                  }
                }
              }
            }
          }

          if ((msg.m_notify.interrupts & irq_set_mouse)) {
            mouse_asm_ih();
            if (!game_not_started) { //Mouse is only used in the game
              if (!startedJump) {
                if (counterByte == 1) {
                  if ((mouseByte & BIT(3)) != 0) //First byte received from the mouse
                    counterByte++;               //Next byte will be the second
                  firstMouseByte = mouseByte;
                }
                else if (counterByte == 2) { //Second byte received from the mouse
                  counterByte++;             //Next byte will be the third
                }
                else {
                  counterByte = 1; //Next byte will be the first
                  if ((firstMouseByte & LB) && !rockyDead) {
                    startedJump = true;
                  }
                }
              }
            }
          }

          if (msg.m_notify.interrupts & irq_set_timer) {
            timer_asm_ih();
            if (!game_not_started) { //Game
              if (!rockyDead) {
                int jumpResult = 0;

                if (counter % 4 == 0) { //Every 4 interrupts, it makes the jump
                  if (startedJump) {
                    jumpResult = makeJump(jump);
                  }

                  if (check_collisions(jump, sizeof(enemies_on_the_screen) / sizeof(Sprite *), enemies_on_the_screen, startedJump)) { //Check jump collisions
                    dead = get_dead_sprite(5, jump, noTongue);
                    rockyDead = true;
                  }
                }

                clear_game_screen(isDay, first_time_drawing_background);
                draw_sprite(star);
                draw_sprite(scorexpm);

                if ((counter >= 500) && (counter % time_to_create_enemy == 0)) { //Create enemies
                  uint8_t rand_pos = rand() % (sizeof(enemies) / sizeof(Sprite *) + 1);
                  if (rand_pos == 3) { //It's the dragon
                    dragon->y = star->y + star->height + 20 + rand() % 80;
                  }
                  else if (rand_pos == 4) {
                    bird->y = star->y + star->height + 20 + rand() % 80;
                  }

                  if (rand_pos < sizeof(enemies) / sizeof(Sprite *)) {
                    put_enemie_on_position(sizeof(enemies_on_the_screen) / sizeof(Sprite *), enemies_on_the_screen, enemies[rand_pos]);
                  }
                }

                if ((counter != 0) && (counter % 5000 == 0)) { //Increase speed
                  bush->xspeed -= 2;
                  spike_monster->xspeed -= 2;
                  spike->xspeed -= 2;
                  dragon->xspeed -= 4;
                  bird->xspeed -= 1;

                  jump->yspeed += 5;
                  if (abs(jump->yspeed) <= 5)
                    jump->yspeed = 5;

                  rocky_animation->aspeed--;
                  if (rocky_animation->aspeed < 1)
                    rocky_animation->aspeed = 1;

                  if (rocky_animation->cur_aspeed > rocky_animation->aspeed)
                    rocky_animation->cur_aspeed = rocky_animation->aspeed;

                  time_to_create_enemy -= 20;
                  if (time_to_create_enemy <= 20) {
                    time_to_create_enemy += 20 - 1;
                    if (time_to_create_enemy < 5)
                      time_to_create_enemy = 5;
                  }
                }

                update_enemies_positions(sizeof(enemies_on_the_screen) / sizeof(Sprite *), enemies_on_the_screen); //Update enemies

                draw_enemies(sizeof(enemies_on_the_screen) / sizeof(Sprite *), enemies_on_the_screen); //Draw all enemies

                if (counter % 10 == 0) { //Every 10 interrupts, the score is incremented
                  score++;
                }

                if (jumpResult == -1)
                  return 1;
                else if (jumpResult == 0 && startedJump) {
                  draw_sprite(jump);
                }
                else if (jumpResult == 1) {
                  draw_sprite(initial);
                  startedJump = false;
                }
                else if (!startedJump) {
                  if (animate_animSprite(rocky_animation)) //Animates the character
                    return 1;

                  if (check_collisions(jump, sizeof(enemies_on_the_screen) / sizeof(Sprite *), enemies_on_the_screen, startedJump)) { //Check collisions
                    dead = get_dead_sprite(get_sprite_number(rocky_animation), rocky_animation->sp, noTongue);
                    rockyDead = true;
                  }
                }

                if (rockyDead) {
                  clear_game_screen(isDay, first_time_drawing_background); //Clear screen with the background
                  draw_sprite(star);
                  draw_sprite(scorexpm);

                  draw_enemies(sizeof(enemies_on_the_screen) / sizeof(Sprite *), enemies_on_the_screen);
                  draw_sprite(dead);
                  Sprite *game_over = create_sprite(game_over_xpm, 0, 0, 0, 0);
                  game_over->x = (int) ((double) get_h_res() / 2 - (double) game_over->width / 2);
                  game_over->y = star->height;
                  jumpResult = false;
                  draw_sprite(game_over);
                  destroy_sprite(game_over);
                  arrow->x = 205;
                  draw_sprite(arrow);

                  put_highscore(score, sizeof(high_scores) / sizeof(high_scores[0]), high_scores); //If Rocky deads, the score will be putted in the array
                }

                if (draw_score(score))
                  return 1;

                copyToVRAM();
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

  if (mouse_unsubscribe_int())
    return 1;

  if (kbd_unsubscribe_int()) { //Unsubscribing keyboard interrupts
    return 1;
  }

  if (timer_unsubscribe_int()) //Unsubscribing timer interrupts
    return 1;

  //Destroys all the Sprites

  destroy_sprite(initial);
  destroy_sprite(jump);
  destroy_sprite(other_jump);

  destroy_animSprite(rocky_animation);

  destroy_sprite(star);
  destroy_sprite(scorexpm);
  destroy_sprite(dead);
  destroy_sprite(arrow);

  destroy_sprite(bush);
  destroy_sprite(spike_monster);
  destroy_sprite(spike);
  destroy_sprite(dragon);
  destroy_sprite(bird);

  destroy_sprite(name);

  save_high_scores(sizeof(high_scores) / sizeof(high_scores[0]), high_scores, 20, player_name); //Save scores in the file

  return 0;
}

int drawBackground(bool isDay, bool first_time_drawing_background) { //Draw game background
  char *second_buffer = get_second_buffer();

  if (first_time_drawing_background) { //First time drawing background, draws pixel by pixel
    if (isDay) {
      if (vg_draw_rectangle(0, 0, get_h_res(), 417, SKY))
        return 1;
    }
    else {
      if (vg_draw_rectangle(0, 0, get_h_res(), 417, NIGHT_SKY))
        return 1;
    }

    if (vg_draw_rectangle(0, 417, get_h_res(), 23, GRASS))
      return 1;
    if (vg_draw_rectangle(0, 440, get_h_res(), get_v_res() - 440, GROUND))
      return 1;

    memcpy(background, second_buffer, get_h_res() * get_v_res() * get_numBytes());
  }
  else { //Other times, copy background to second buffer
    memcpy(second_buffer, background, get_h_res() * get_v_res() * get_numBytes());
  }
  return 0;
}

int draw_score(uint32_t score) { //Draw score in the second buffer
  int x = 220;
  Sprite *num;
  if (score != 0) {
    uint32_t tmp_score = score;
    uint32_t score_num[6] = {0};

    uint8_t pos = 5;

    while (tmp_score != 0) { //Put score in an array
      score_num[pos] = tmp_score % 10;
      tmp_score /= 10;
      pos--;
    }

    pos++;

    while (pos < 6) {
      switch (score_num[pos]) { //Creates Sprite for number
        case 0:
          num = create_sprite(zero_xpm, x, 20, 0, 0);
          break;
        case 1:
          num = create_sprite(one_xpm, x, 20, 0, 0);
          break;
        case 2:
          num = create_sprite(two_xpm, x, 20, 0, 0);
          break;
        case 3:
          num = create_sprite(three_xpm, x, 20, 0, 0);
          break;
        case 4:
          num = create_sprite(four_xpm, x, 20, 0, 0);
          break;
        case 5:
          num = create_sprite(five_xpm, x, 20, 0, 0);
          break;
        case 6:
          num = create_sprite(six_xpm, x, 20, 0, 0);
          break;
        case 7:
          num = create_sprite(seven_xpm, x, 20, 0, 0);
          break;
        case 8:
          num = create_sprite(eight_xpm, x, 20, 0, 0);
          break;
        default: //9
          num = create_sprite(nine_xpm, x, 20, 0, 0);
          break;
      }
      if (draw_sprite(num)) //Draw number in the second buffer
        return 1;
      x += 35;
      pos++;
      destroy_sprite(num);
    }
  }
  else {
    num = create_sprite(zero_xpm, x, 20, 0, 0);
    if (draw_sprite(num))
      return 1;
    destroy_sprite(num);
  }

  return 0;
}

//Util functions for timer_set_frequency()
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

void free_game_background() { //Free game background
  free(background);
}

void game_background_alloc() { //Allocate memory to the game background
  background = malloc(get_h_res() * get_v_res() * get_numBytes());
}
