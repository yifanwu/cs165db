#ifndef LIST_H_
#define LIST_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

typedef struct
{
  uint32_t unit;
  uint32_t next;
  uint32_t capacity;
  void *data;
} list_t;

list_t *create_list_of_size(uint32_t size, uint32_t unit);
list_t *create_list(uint32_t unit);
void *append_list(list_t *list, void *val);
void destroy_list(list_t *list);
uint32_t get_list_size(list_t *list);
void *get_list_val(list_t *list, uint32_t index);

#endif //LIST_H_
