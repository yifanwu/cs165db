#ifndef BIT_VECTOR_H_
#define BIT_VECTOR_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum {MARK, UNMARK} bit_op;

typedef struct
{
  uint32_t size;
  uint32_t count;
  uint32_t *vec;
} bv_t;

bv_t *create_bv(uint32_t size);
void destroy_bv(bv_t *bv);

bool is_marked(bv_t *bv, uint32_t index);

int mark_bv(bv_t *bv, uint32_t index);
int unmark_bv(bv_t *bv, uint32_t index);
int toggle_bv(bv_t *bv, uint32_t index);
void mark_bv_by_range(bv_t *bv, uint32_t low, uint32_t high);

void mark_all_bv(bv_t *bv);
void unmark_all_bv(bv_t *bv);
void _mark_all_bv_by_op(bv_t *bv, bit_op op);

// helper
uint32_t get_bv_count(bv_t *bv);
uint32_t get_bv_size(bv_t *bv);
void cpy_bv(bv_t *des, bv_t *src);
void print_binary(uint32_t n);

#endif //BIT_VECTOR_H_
