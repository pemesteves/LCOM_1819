#include "sprite.h"
#include "lcom/pixmap.h"
#include <lcom/lcf.h>
#include <stdint.h>
#include <stdio.h>

/** Creates a new sprite with pixmap "pic", with specified
* position (within the screen limits) and speed;
* Does not draw the sprite on the screen
* Returns NULL on invalid pixmap.
*/
Sprite *create_sprite(const char *pic[], int x, int y, int xspeed, int yspeed) {
  //allocate space for the "object"
  Sprite *sp = (Sprite *) malloc(sizeof(Sprite));

  if (sp == NULL)
    return NULL;

  // read the sprite pixmap
  sp->x = x;
  sp->y = y;
  sp->xspeed = xspeed;
  sp->yspeed = yspeed;
  sp->map = read_xpm(pic, &(sp->width), &(sp->height));

  if (sp->map == NULL) {
    free(sp);
    return NULL;
  }

  return sp;
}

void destroy_sprite(Sprite *sp) {
  if (sp == NULL)
    return;

  if (sp->map)
    free(sp->map);

  free(sp);
  sp = NULL; // XXX: pointer is passed by value
             // should do this @ the caller
}

int animate_sprite(Sprite *UNUSED(sp)) {
  //...

  printf("function not implemented");
  return 1;
}

/* Some useful non-visible functions */
//static int draw_sprite(Sprite *sp, char *base) {
//...
//}
//static int check_collision(Sprite *sp, char *base) {
//...
//}
