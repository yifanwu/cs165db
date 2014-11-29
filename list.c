/****
 * Yifan's dynamic array implementation!
 ****/
#include "list.h"
#include "dbg.h"
#include "config.h"


list_t *create_list_of_size(uint32_t size, uint32_t unit)
{
  list_t *list = malloc(sizeof(list_t));
  list->next = 0;
  list->unit = unit;
  list->capacity = (size + 1) << 1;
  list->data = malloc(unit * list->capacity);
  assert( list->data != NULL);
  return list;
}

list_t *create_list(uint32_t unit)
{
  return create_list_of_size(DEFAULT_DYN_ARR_SIZE, unit);
}

uint32_t get_list_size(list_t *list)
{
  return list->next;
}

void *get_list_val(list_t *list, uint32_t index)
{
  void *ptr = &((char *)list->data)[list->unit * index];
  return ptr;
}

void *append_list(list_t *list, void *val)
{
  if (list->next == list->capacity) {
    list->capacity <<= 1;
    list->data = realloc(list->data, list->unit * list->capacity);
  }
  void *ptr = &((char *)list->data)[list->unit * list->next];
  //debug("appended %d", *(int *)val);
  memcpy(ptr, val, list->unit);
  list->next ++;
  return ptr;
}

/* //TODO: implement (but no use yet)
void cpy_list(list_t *dst, list_t *src)
{
  // assume it's already allocated

}*/

void destroy_list(list_t *list)
{
  free(list->data);
  free(list);
}
