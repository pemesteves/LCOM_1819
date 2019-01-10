// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>
// Any header files included below this line should have been created by you
#include "game.h"
#include "menu.h"
#include "proj.h"
#include "videoCard.h"

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("PT-PT");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/proj/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/proj/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(proj_main_loop)(int UNUSED(argc), char *UNUSED(argv[])) { //Project Main Loop
  sys_enable_iop(SELF); //Enable assembly functions

  if (vg_init(DIRECT_COLOR_MODE_888) == NULL) //Set video mode
    return 1;

  menu_background_alloc(); //Allocating memory to menu background
  game_background_alloc(); //Allocating memory to game background

  int8_t option = 1;

  do {
    option = menu(); //Menu function

    if (option == 1) {
      if (game(ROCKY_START_POINT_X, ROCKY_START_POINT_Y, FRAMES_PER_SECOND)) //Game function
        option = -1;
    }
  } while (option != -1);

  freeBuffers(); //Free video memory and second buffer
  free_menu_background(); //Free menu background
  free_game_background(); //Free game background

  if (vg_exit()) //Exit from video mode
    return 1;

  return 0;
}
