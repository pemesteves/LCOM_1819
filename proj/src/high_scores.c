#include <lcom/lcf.h>
#include <math.h>
// Any header files included below this line should have been created by you
#include "high_scores.h"
#include "menu.h"
#include "proj.h"
#include "videoCard.h"
#include "xpm.h"

static char *high_scores_screen;

static void swap_strings(char *str1, char *str2) { //Swap two strings (used in the player names)
  char *temp = (char *) malloc((strlen(str1) + 1) * sizeof(char));
  strcpy(temp, str1);
  strcpy(str1, str2);
  strcpy(str2, temp);
  free(temp);
}

void put_highscore(uint32_t score, uint8_t numElem, uint32_t high_scores[]) { //Put score in the highscores array
  uint32_t min_score = score;
  uint8_t pos_min_score = numElem;
  bool found_place = false;
  for (uint8_t i = 0; i < numElem; i++) { //Put highscore in the mininum score place
    if (high_scores[i] == 0) {
      high_scores[i] = score;
      found_place = true;
      break;
    }
    else if (high_scores[i] < min_score) {
      min_score = high_scores[i];
      pos_min_score = i;
    }
  }

  if (!found_place) {
    if (pos_min_score != numElem) {
      if (score > high_scores[pos_min_score])
        high_scores[pos_min_score] = score;
    }
  }

  //Now we have to order the array
  for (uint8_t i = 0; i < numElem; i++) {
    for (uint8_t j = 0; j < numElem; j++) {
      if (high_scores[j] < high_scores[i]) {
        uint32_t tmp = high_scores[i];
        high_scores[i] = high_scores[j];
        high_scores[j] = tmp;
      }
    }
  }
}

void save_high_scores(uint8_t numElem, uint32_t high_scores[], uint8_t name_size, char player_name[]) { //Save highscores in a file

  FILE *fptr;
  fptr = (fopen("/home/lcom/labs/proj/src/high_scores/high_scores.txt", "r+")); //Opening file for reading

  uint32_t file_scores[10] = {0};
  char names[10][13] = {""};
  uint8_t num_score = 0;

  while (fscanf(fptr, "%s %u", names[num_score], &file_scores[num_score]) == 2 && num_score < 10) { //Import previous highscores and names
    num_score++;
  }

  fclose(fptr);

  uint8_t min_position = 0;

  for (uint8_t i = 0; i < numElem; i++) {
    min_position = 0;
    for (uint8_t j = 0; j < sizeof(file_scores) / sizeof(file_scores[0]); j++) { //10 = size of file_scores
      if (file_scores[j] < file_scores[min_position]) {
        min_position = j;
      }
    }

    if (high_scores[i] > file_scores[min_position]) {
      file_scores[min_position] = high_scores[i];

      for (uint8_t k = 0; k < sizeof(names[min_position]) / sizeof(names[min_position][0]); k++) {
        if (k > name_size)
          names[min_position][k] = ' ';
        else
          names[min_position][k] = player_name[k];
      }
    }
    else
      break;
  }

  //Now we have to order the array
  for (uint8_t i = 0; i < numElem; i++) {
    for (uint8_t j = 0; j < numElem; j++) {
      if (file_scores[j] < file_scores[i]) {
        uint32_t tmp = file_scores[i];
        file_scores[i] = file_scores[j];
        file_scores[j] = tmp;
        swap_strings(names[i], names[j]);
      }
    }
  }

  fptr = (fopen("/home/lcom/labs/proj/src/high_scores/high_scores.txt", "w+")); //Opening the same file for writing (this prevents some possible errors)

  num_score = 0;
  while (num_score < 10) {
    if (file_scores[num_score] == 0) //Only ROCKY can have a score with 0 points
      strcpy(names[num_score], "ROCKY");

    fprintf(fptr, "%s %u\n", names[num_score], file_scores[num_score]);
    num_score++;
  }

  /*SAVES THE FILE:

    PEDRO 12345
    PEDRO 1234
    ...

    10 ELEMENTS MAX
  */

  fclose(fptr);
}

void draw_high_scores(bool firstTime) { //Draw highscores in the screen
  if (firstTime) {

    //Highscores xpm
    Sprite *highscores = create_sprite(highscores_title_xpm, 0, 103, 0, 0);
    highscores->x = (int) ((double) get_h_res() / 2.0 - (double) highscores->width / 2.0);
    draw_sprite(highscores);

    FILE *fptr;
    fptr = (fopen("/home/lcom/labs/proj/src/high_scores/high_scores.txt", "r")); //Opening file only for reading
    if (fptr == NULL) {                                                          //If fptr is NULL, there is no data to read
      return;
    }

    uint32_t file_scores[10] = {0};
    char names[10][13] = {""};
    uint8_t num_score = 0;
    while ((fscanf(fptr, "%s %u", names[num_score], &file_scores[num_score]) == 2) && (num_score < 10)) { //Reading file
      num_score++;
    }
    fclose(fptr);

    Sprite *character;
    int y = highscores->y + highscores->height + 10;
    int x = 20;

    for (uint8_t i = 0; i < 10; i++) { //Drawing screen
      x = 20;
      if (i < 9) {
        character = get_character('0' + i + 1);
        if (character != NULL) {
          character->x = x;
          character->y = y;
          draw_sprite(character);
          destroy_sprite(character);
        }
        x += 70;
      }
      else {
        character = get_character('1');
        if (character != NULL) {
          character->x = x;
          character->y = y;
          draw_sprite(character);
          destroy_sprite(character);
        }
        x += 30;
        character = get_character('0');
        if (character != NULL) {
          character->x = x;
          character->y = y;
          draw_sprite(character);
          destroy_sprite(character);
        }
        x += 40;
      }

      for (uint8_t j = 0; j < 13; j++) {
        character = get_character(names[i][j]);
        if (character != NULL) {
          character->x = x;
          character->y = y;
          draw_sprite(character);
          destroy_sprite(character);
        }
        x += 30;
      }
      x += 30;

      uint32_t score = file_scores[i];
      uint8_t num_digits = 0;
      if (score == 0)
        num_digits = 1;

      while (score != 0) {
        score /= 10;
        num_digits++;
      }

      score = file_scores[i];

      for (uint8_t j = 0; j < num_digits; j++) {
        character = get_character('0' + score / (uint32_t) pow(10, num_digits - j - 1));

        if (character != NULL) {
          character->x = x;
          character->y = y;
          draw_sprite(character);
          destroy_sprite(character);
        }

        score = score % (uint32_t) pow(10, num_digits - j - 1);
        x += 30;
      }

      y += 35;
    }

    char *second_buffer = get_second_buffer();
    memcpy(high_scores_screen, second_buffer, get_h_res() * get_v_res() * get_numBytes()); //Copy second buffer to high_scores_screen

    destroy_sprite(highscores);
  }
  else {
    char *second_buffer = get_second_buffer();
    memcpy(second_buffer, high_scores_screen, get_h_res() * get_v_res() * get_numBytes()); //Copy high_scores_screen content to second buffer
  }
}

void alloc_high_scores() { //Allocating space for the high_scores_screen
  high_scores_screen = malloc(get_h_res() * get_v_res() * get_numBytes());
}

void free_high_scores() { //Free high_scores_screen
  free(high_scores_screen);
}
