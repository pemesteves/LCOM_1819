#include <lcom/lcf.h>

#include "animSprite.h"

AnimSprite *create_animSprite(uint16_t xi, uint16_t yi, uint8_t no_pic, xpm_string_t pic1[], ...) {
  AnimSprite *asp = malloc(sizeof(AnimSprite)); // create a standard sprite with first pixmap
  asp->sp = create_sprite(pic1, xi, yi, 0, 0);  // allocate array of pointers to pixmaps
  asp->map = malloc((no_pic) * sizeof(char *)); // initialize the first pixmap
  asp->map[0] = asp->sp->map;                   // continues in next transparency

  // initialize the remainder with the variable arguments
  // iterate over the list of arguments
  va_list ap;
  va_start(ap, pic1);
  unsigned i;
  int w, h;
  for (i = 1; i < no_pic; i++) { //Create array of pixmaps
    xpm_string_t *tmp = va_arg(ap, xpm_string_t *);
    xpm_image_t img;
    if (xpm_load(tmp, XPM_8_8_8, &img) == NULL)
      return NULL;

    h = img.height;
    w = img.width;

    asp->map[i] = img.bytes;
    if (asp->map[i] == NULL || w != asp->sp->width || h != asp->sp->height) { // failure: realease allocated memory
      for (unsigned j = 1; j < i; j++)
        free(asp->map[i]);
      free(asp->map);
      destroy_sprite(asp->sp);
      free(asp);
      va_end(ap);
      return NULL;
    }
  }
  va_end(ap);

  asp->aspeed = 8;     // no. frames per pixmap
  asp->cur_aspeed = 0; // no. frames left to next change
  asp->num_fig = i;    // number of pixmaps
  asp->cur_fig = 0;    // current pixmap

  return asp;
}

int animate_animSprite(AnimSprite *sp) { //Function that animates the character
  sp->cur_aspeed++;
  if (sp->cur_aspeed == sp->aspeed) {
    sp->cur_aspeed = 0;
    sp->cur_fig++;
    if (sp->cur_fig == sp->num_fig)
      sp->cur_fig = 0;
    sp->sp->map = *(sp->map + sp->cur_fig);
  }
  if (draw_sprite(sp->sp))
    return 1;
  return 0;
}

void destroy_animSprite(AnimSprite *sp) { //Destroy the AnimSprite struct
  destroy_sprite(sp->sp);
  free(sp->map);
  free(sp);
}

int get_sprite_number(AnimSprite* sp){ //Returns the number of the Sprite used to get the corresponding dead Sprite
  return sp->cur_fig;
}

void swap_animSprites(AnimSprite* sp1, AnimSprite* sp2){ //Swaps two Animated Sprites
  uint8_t **tmp_map = sp1->map;
  sp1->map = sp2->map;
  sp2->map = tmp_map;
}

