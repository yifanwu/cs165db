#include "client.h"

#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "dbg.h"
#include "config.h"
#include "helper.h"

/*
 * very hacky implementation
 * might add more robustness later
 */
char *parse_load_filename(char *command) {

  size_t file_name_size = strlen(command)-8;
  if (command[strlen(command)-1] == '\n') {
    file_name_size--;
  }
  char *file_name = malloc(sizeof(char) * (file_name_size + 1));
  memcpy(file_name, &command[6], file_name_size);
  file_name[file_name_size] = '\0';
  return file_name;
}

/***********************************************************
 *
 * READ by line and place in the array of allocated space
 * assuming the descriptor is in the correct place
 *
 * get char should hopefully not be too slow since the OS
 * should do some buffering and we have locality!
 * UNIT TESTED
 **********************************************************/

int get_name_line(FILE *f, char *data, uint32_t size)
{
  int ch;
  uint32_t item_counter = 0;
  uint32_t char_counter = 0;
  do
  {
    ch = fgetc(f);
    if ( ch == '\n' || ch == -1) {
      // NULL terminate
      data[item_counter * MAX_NAME_SIZE + char_counter] = '\0';
      assert (item_counter == size - 1);
      return 0;
    }
    else if ( ch == ',' ) {
      // NULL terminate
      data[item_counter * MAX_NAME_SIZE + char_counter] = '\0';
      item_counter ++;
      char_counter = 0;
    }
    else {
      data[item_counter * MAX_NAME_SIZE + char_counter] = (char) ch;
      char_counter ++;
    }
  } while(item_counter < size);

  // shouldn't be here
  assert(false);
  return 1;
}

int get_int_line(FILE *f, int *data, uint32_t size)
{
  int ch;
  uint32_t item_counter = 0;
  uint32_t char_counter = 0;
  char holder[INT_STR_WIDTH];
  while(!feof(f)) {
    ch = fgetc(f);
    // end of file
    if ( ch == '\n' || ch == -1) {
      if (item_counter == 0) {
        return 0;
      }
      holder[char_counter] = '\0';
      data[item_counter] = (int)strtol(holder, NULL, 10);
      verbose("NUM %d\n", data[item_counter]);
      assert (item_counter == size - 1);
      return 0;
    }
    else if ( ch == ',' ) {
      // NULL terminate
      holder[char_counter] = '\0';
      data[item_counter] = (int)strtol(holder, NULL, 10);
      item_counter ++;
      char_counter = 0;
    }
    else {
      holder[char_counter] = (char) ch;
      char_counter ++;
    }
  }

  // shouldn't be here
  assert(false);
  return 1;

}






