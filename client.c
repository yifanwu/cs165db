#include "client.h"
#define _XOPEN_SOURCE 700

#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close
#include "dbg.h"
#include "config.h"
#include "helper.h"

int main(void)
{
  int s, result;
  size_t len;
  struct sockaddr_un remote;
  // used for communication with server this should fit on the stack
  char data_buffer[MAX_COMMAND_BUFFER_SIZE];
  char *line = NULL;
  char str_for_int[20];
  char msg[MSG_SIZE];
  size_t row_expected;
  size_t size_expected;
  size_t getline_num = FILE_BLOCK_SIZE;

  // declaired globally so that we could free them together should there be an error
  char *col_names = NULL; // for saving the column names from file
  int *int_data = NULL; // for saving the file ints and also getting results
  char *v = NULL; // storing the variables

  if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    exit(1);
  }

  verbose("Connecting...");

  remote.sun_family = AF_UNIX;
  strcpy(remote.sun_path, SOCK_PATH);
  len = strlen(remote.sun_path) + sizeof(remote.sun_family);
  if (connect(s, (struct sockaddr *)&remote, len) == -1) {
    perror("connect");
    exit(1);
  }

  while(!feof(stdin)) {
    //{{{ while loop for client

    getline(&line, &getline_num, stdin);
    len = strlen(line);
    if (len == 0) {
      debug("LEN IS ZERO %s", line);
      goto done;
    }
    if (line[len-2] !=')') {
      // TODO: this is a HACK, need to find out why
      debug("some crap in buffer: %s, which ended in %c", line, line[len-2]);
      goto done;
    }
    debug("Command is: %s", line);

    /************
     * LOAD
     ************/
    if (strncmp(line, "load", 4) == 0) {
      // load command, first parse file name
      char *file_name = parse_load_filename(line);

      if (file_name == NULL) {
        perror("file name");
        exit(EXIT_FAILURE);
      }

      // then get the file descriptor
      FILE *load_fp = fopen(file_name, "r");
      if (load_fp == NULL) {
        log_err("Cannot find file %s", file_name);
        perror("file name");
        exit(1);
      }
      free(file_name);

      // now get how many rows & columns there are
      fseek(load_fp, 0, SEEK_SET);
      int ch;
      uint32_t row_count = 0;
      uint32_t col_count = 0;
      do
      {
        ch = fgetc(load_fp);
        if ( ch == '\n') {
          // note that this is naturally one shorter
          // so exactly number of int rows
          row_count ++;
        }
        else if (row_count == 0 && ch == ',') {
          col_count ++;
        }
      } while( ch != EOF );

      fseek(load_fp, -1, SEEK_END);
      ch = fgetc(load_fp);
      if (ch == '\n') {
        row_count --;
      }

      // one off
      col_count ++;

      verbose("Column count: %d, Row count: %d\n", col_count, row_count);
      // now send the data over
      strcpy(msg, "load(");
      sprintf(str_for_int, "%d", col_count);
      strcat(msg, str_for_int);
      strcat(msg, ",");
      sprintf(str_for_int, "%d", row_count);
      strcat(msg, str_for_int);
      strcat(msg, ")");
      result = safe_send(s, msg, strlen(msg));
      if (result) goto done;

      // now we need to actually restructure the data
      // no dynamic array needed though
      uint32_t total_bytes = sizeof(char) * MAX_NAME_SIZE * col_count;
      col_names = malloc(total_bytes);
      check_mem(col_names);
      fseek(load_fp, 0, SEEK_SET);
      get_name_line(load_fp, col_names, col_count);

      result = safe_send(s, col_names, total_bytes);
      if (result) goto done;

      free(col_names);

      total_bytes = sizeof(int) * col_count;
      int_data = malloc(total_bytes);
      for (uint32_t i = 0; i < row_count; i++) {
        get_int_line(load_fp, int_data, col_count);
        result = safe_send(s, (char *) int_data, total_bytes);
        if (result) goto done;
      }
      free(int_data);
      fclose(load_fp);
      continue;
    }

    /************
     * TUPLE
     * DELETE
     * INSERT
     ************/
    else if ((strncmp(line, "tuple", 5) == 0) || (strncmp(line, "delete", 6) == 0) ||
      (strncmp(line, "insert", 6) == 0) )
    {

      int offset;
      switch(line[0]) {
        case 't':
          // because tuple( is 6 char
          offset = 6;
          break;
        default:
          // because delete( and insert( are both 7 char
          offset = 7;
          break;
      }
      char * token = strtok (&(line[offset]), ",)");
      // go and get all the variables!
      uint32_t v_count = 0;
      uint32_t v_buffer = (1 + v_count) << 1;

      v = malloc(sizeof(char) * MAX_NAME_SIZE * v_buffer);
      while (token != NULL)
      {
        verbose("Token is %s\n", token);
        if(v_count == v_buffer) {
          v_buffer = (1 + v_count) << 1;
          v = realloc(v, sizeof(char) * MAX_NAME_SIZE * v_buffer);
        }
        strcpy(&(v[v_count * MAX_NAME_SIZE]), token);
        v_count ++;
        token = strtok (NULL, ",)");
      }

      assert(v_count > 0);
      v_count --; // over counted by one before!

      strncpy(msg, line, offset);
      //TODO: remove
      debug("The line is %s, and offset %d\n", line, offset);
      dbg_assert(offset < 8);
      sprintf(&(msg[offset]), "%d", v_count);
      //strcat(msg, str_for_int);
      strcat(msg, ")");

      result = safe_send(s, msg, strlen(msg));
      if (result) goto done;
      // now send data over
      debug("Sending instructions over, total size is %u!\n", (uint32_t)sizeof(char) * MAX_NAME_SIZE * v_count);
      result = safe_send(s, v, sizeof(char) * MAX_NAME_SIZE * v_count);
      if (result) goto done;
      // free after finished
      free(v);

      if (line[0] == 't') {
        // now receive data!
        debug("Now waiting for socket to get back!\n");

        result = safe_recv(s, data_buffer, sizeof(socket_meta_t));
        if (result ) goto done;
        size_expected = ((socket_meta_t *)data_buffer)->expected_bytes;
        int_data = malloc(size_expected);

        row_expected = size_expected/(sizeof(int) * v_count);
        debug("we expect %d columns and %zu rows", v_count, row_expected);

        result = safe_recv(s, (char *)int_data, size_expected);
        if (result ) goto done;

        // these are supposed to be printed out
        // DON'T DELETE YO
        for(uint32_t i = 0; i < row_expected; i++) {
          printf("(");
          for(uint32_t j = 0; j < v_count; j++) {
            printf("%d", ((int *)int_data)[j * row_expected + i]);
            if(j != v_count-1) {
              printf(",");
            }
          }
          printf(")\n");
        }
      }
      continue;
    }

    /*******************
     * OTHER COMMANDS
     *******************/
    else {
      result = safe_send(s, line, len);
      if (result) goto done;

      /************
       * fetch
       ************/
      if (strncmp(line, "fetch", 5) == 0) {

        verbose("Block waiting on fetch.\n");
        safe_recv(s, data_buffer, sizeof(socket_meta_t));
        size_expected = ((socket_meta_t *)data_buffer)->expected_bytes;
        int_data = malloc(size_expected);
        check_mem(int_data);
        result = safe_recv(s, (char *)int_data, (uint32_t)size_expected);
        if (result) goto done;
        row_expected = size_expected/(sizeof(int));
        verbose("FETCH RESULT: row_expected is %zu, size is %zu", row_expected, size_expected);
        for(uint32_t i = 0; i< row_expected; i++) {
          printf("%d\n", ((int *)int_data)[i]);
        }
        free(int_data);
      }
    }
  } //}}}

done:
  verbose("CLOSING connection\n");
  close(s);

  return 0;
error:
  if(int_data) free (int_data);
  if(col_names) free(col_names);
  if(v) free (v);
  return 1;
}


