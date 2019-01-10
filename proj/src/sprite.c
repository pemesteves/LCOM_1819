#include <lcom/lcf.h>

#include "game.h"
#include "proj.h"
#include "sprite.h"
#include "videoCard.h"
#include "xpm.h"
#include <math.h>
#include <stdarg.h> // va_* macros are defined here
#include <stdint.h>
#include <stdio.h>

//static int clearSprite(int x, int y, int width, int height);

static uint8_t jumpPosition = 0;

/** Creates a new sprite with pixmap "pic", with specified
* position (within the screen limits) and speed;
* Does not draw the sprite on the screen
* Returns NULL on invalid pixmap.
*/
Sprite *create_sprite(const xpm_string_t pic[], int x, int y, int xspeed, int yspeed) {
  //allocate space for the "object"
  Sprite *sp = (Sprite *) malloc(sizeof(Sprite));

  if (sp == NULL)
    return NULL;

  // read the sprite pixmap
  sp->x = x;
  sp->y = y;
  sp->xspeed = xspeed;
  sp->yspeed = yspeed;

  xpm_image_t img;
  if (xpm_load(pic, XPM_8_8_8, &img) == NULL)
    return NULL;

  sp->height = img.height;
  sp->width = img.width;
  sp->map = img.bytes;

  if (sp->map == NULL) {
    free(sp);
    return NULL;
  }

  return sp;
}

void destroy_sprite(Sprite *sp) { //Destroy Sprite
  if (sp == NULL)
    return;

  if (sp->map)
    free(sp->map);

  free(sp);
  sp = NULL; // XXX: pointer is passed by value
             // should do this @ the caller
}

int makeJump(Sprite *sp) { //Makes the character jump

  switch (jumpPosition) { //If the speed is negative, the sprite is jumping
    case 0: //Initial jump position
      if (sp->yspeed > 0) {
        sp->y = ROCKY_START_POINT_Y;
        sp->yspeed = -sp->yspeed;
        return 1; //It means that the jump is finished
      }
      else {
        sp->y += sp->yspeed;
        jumpPosition++;
      }
      break;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5: //Final jump position
      sp->y += sp->yspeed;
      if (sp->yspeed > 0) {
        jumpPosition--;
      }
      else {
        jumpPosition++;
      }
      break;
    case 6:
      sp->yspeed = -sp->yspeed;
      jumpPosition--;
      break;
    default:
      break;
  }
  if (draw_sprite(sp))
    return -1; //Occurred an error

  return 0;
}

int clear_game_screen(bool isDay, bool first_time_drawing_background) { //Clear game screen
  char *second_buffer = get_second_buffer();
  memset(second_buffer, 0, get_h_res() * get_v_res() * get_numBytes()); //Put screen in black
  if (drawBackground(isDay, first_time_drawing_background)) //Draw background
    return 1;
  return 0;
}

int draw_sprite(Sprite *sp) { //Draw a Sprite pixel by pixel in the second buffer
  if (sp->x >= (int) get_h_res() || sp->y >= (int) get_v_res())
    return 0;
  uint8_t *map = sp->map;
  unsigned num_bytes = get_numBytes();
  uint32_t color;
  for (int y = 0; y < sp->height; y++) {
    for (int x = 0; x < sp->width; x++) {
      color = 0;
      for (unsigned byte = 0; byte < num_bytes; byte++) {
        color += *(map + byte) << byte * 8;
      }
      if (drawPixel(sp->x + x, sp->y + y, color))
        return 1;

      map += num_bytes;
    }
  }

  return 0;
}

static int check_collision(Sprite *sp, Sprite *enemy, bool jumping) { //Checks collision with an enemy
  double sp_center_x = sp->x + (double) sp->width / 2;
  double sp_center_y = sp->y + (double) sp->height / 2;
  double enemy_center_x = enemy->x + (double) enemy->width / 2;
  double enemy_center_y = enemy->y + (double) enemy->height / 2;

  double distance_x = abs(enemy_center_x - sp_center_x);
  double distance_y = abs(enemy_center_y - sp_center_y);

  double tolerance = 0;

  if (jumping)
    tolerance = 20;
  else
    tolerance = 0;

  if (distance_x < abs((double) enemy->width / 2 + (double) sp->width / 2) - tolerance && distance_y < abs((double) enemy->height / 2 + (double) sp->height / 2) - tolerance) {
    return 1;
  }

  return 0;
}

Sprite *get_dead_sprite(int position, Sprite *sp, bool noTongue) { //Returns the dead Sprite corresponding to the current animation
  Sprite *newSprite;
  switch (position) {
    case 0: //START
      if (noTongue)
        newSprite = create_sprite(no_tongue_dead_start_xpm, sp->x, sp->y, 0, 0);
      else
        newSprite = create_sprite(dead_start_xpm, sp->x, sp->y, 0, 0);
      break;
    case 2: //RUN
      if (noTongue)
        newSprite = create_sprite(no_tongue_dead_run_xpm, sp->x, sp->y, 0, 0);
      else
        newSprite = create_sprite(dead_run_xpm, sp->x, sp->y, 0, 0);
      break;
    default: //JUMP
      if (noTongue)
        newSprite = create_sprite(no_tongue_dead_jump_xpm, sp->x, sp->y, 0, 0);
      else
        newSprite = create_sprite(dead_jump_xpm, sp->x, sp->y, 0, 0);
      break;
  }
  return newSprite;
}

void reinitialize_jumpPosition() { //Reinitialize jumpPosition
  jumpPosition = 0;
}

void swap_sprites(Sprite *sp1, Sprite *sp2) { //Swap the content of two Sprites
  uint8_t *tmp_map = sp1->map;
  sp1->map = sp2->map;
  sp2->map = tmp_map;
}

void update_enemies_positions(uint8_t num_enemies, Sprite *enemies[]) { //Update all enemies positions
  for (uint8_t i = 0; i < num_enemies; i++) {
    if (enemies[i] != NULL) {
      if (enemies[i]->x + enemies[i]->width < 0) { //If the enemy exit the screen, it will be deleted
        free(enemies[i]);
        enemies[i] = NULL;
        continue;
      }
      enemies[i]->x += enemies[i]->xspeed;
    }
  }
}

int draw_enemies(uint8_t num_enemies, Sprite *enemies[]) { //Draw all enemies
  for (uint8_t i = 0; i < num_enemies; i++) {
    if (enemies[i] != NULL) {
      if (draw_sprite(enemies[i]))
        return 1;
    }
  }
  return 0;
}

void put_enemie_on_position(uint8_t num_enemies, Sprite *enemies[], Sprite *new_enemy) { //Put an enemy on an available position of the array
  for (uint8_t i = 0; i < num_enemies; i++) {
    if (enemies[i] == NULL) {
      Sprite *new_e = malloc(sizeof(Sprite));
      memcpy(new_e, new_enemy, sizeof(Sprite));
      enemies[i] = new_e;
      break;
    }
  }
}

int check_collisions(Sprite *sp, uint8_t num_enemies, Sprite *enemies[], bool jumping) { //Check collision with all enemies
  for (uint8_t i = 0; i < num_enemies; i++) {
    if (enemies[i] != NULL) {
      if (check_collision(sp, enemies[i], jumping))
        return 1;
    }
  }
  return 0;
}

void destroy_enemies(uint8_t num_enemies, Sprite *enemies[]) { //Destroy all enemies in the array
  for (uint8_t i = 0; i < num_enemies; i++) {
    if (enemies[i] != NULL) {
      free(enemies[i]);
      enemies[i] = NULL;
    }
  }
}

int check_cursor_collisions(Sprite *cursor, Sprite *rectangle) { //Check mouse cursor collisions in the menu
  if (cursor->x < rectangle->x || cursor->x > rectangle->x + rectangle->width)
    return 0;
  if (cursor->y < rectangle->y || cursor->y > rectangle->y + rectangle->height)
    return 0;

  return 1;
}
