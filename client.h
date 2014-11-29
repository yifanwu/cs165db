#ifndef CLIENT_H_
#define CLIENT_H_

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
// processing command
char *parse_load_filename(char *command);


// processing files
int get_name_line(FILE *f, char *data, uint32_t size);
int get_int_line(FILE *f, int *data, uint32_t size);

#endif // CLIENT_H_
