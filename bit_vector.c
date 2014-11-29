#include "bit_vector.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <assert.h>
#include "helper.h"
#include "dbg.h"

// not that this is not immutable
bv_t *create_bv(uint32_t size)
{
  bv_t *bv = calloc(1, sizeof(bv_t));
  bv->size = size;
  bv->count = 0;

  uint32_t bv_size = get_ceil(size, 32);
  bv->vec = calloc(sizeof(uint32_t), bv_size);
  return bv;
}

uint32_t get_bv_size(bv_t *bv)
{
  return bv->size;
}

uint32_t get_bv_count(bv_t *bv)
{
  return bv->count;
}


bool is_marked(bv_t *bv, uint32_t index) {
  uint32_t bv_index = index >> 5;
  uint32_t offset = index - bv_index * 32;
  uint32_t val = bv->vec[bv_index];
  if ((val >> offset) & 1) {
    return true;
  }
  else {
    return false;
  }
}

void cpy_bv(bv_t *des, bv_t *src)
{
  des->size = src->size;
  des->count = src->count;
  memcpy(des->vec, src->vec, get_ceil(src->size, 32) * sizeof(int));
}

int _mark_bv_by_op(bv_t *bv, uint32_t index, bit_op op)
{
  uint32_t bv_index = index >> 5;
  uint32_t offset = index - bv_index * 32;
  uint32_t mask = 1 << offset;

  switch(op) {
    case MARK:
      bv->vec[bv_index] |= mask;
      bv->count ++;
      break;
    case UNMARK:
      bv->vec[bv_index] &= ~mask;
      bv->count --;
      break;
    // case TOGGLE:
    //   bv->vec[bv_index] ^= mask;
    //   break;
    //default:
      // shouldn't be here
      //assert(false);
      //return 1;
  }

  return 0;
}

int mark_bv(bv_t *bv, uint32_t index)
{
  return _mark_bv_by_op(bv, index, MARK);
}

int unmark_bv(bv_t *bv, uint32_t index)
{
  return _mark_bv_by_op(bv, index, UNMARK);
}

//int toggle_bv(bv_t *bv, uint32_t index)
//{
//  return _mark_bv_by_op(bv, index, TOGGLE);
//}

/***
 * marking by range
 *
 * NOTE: erases the vector before marking by range!
 * This is a design choice
 */

void mark_bv_by_range(bv_t *bv, uint32_t low, uint32_t high)
{
  unmark_all_bv(bv);
  uint32_t mask, mask_low, mask_high;
  uint32_t low_index = low >> 5;
  uint32_t high_index = high >> 5;
  uint32_t low_offset = low - low_index * 32;
  uint32_t high_offset = high - high_index * 32;
  mask_low = (1 << low_offset) - 1;
  mask_high = (1 << (high_offset + 1)) - 1;

  if (low_index == high_index) {
    mask = mask_high - mask_low;
    bv->vec[low_index] |= mask;
  }
  else {
    // now we have to mark everything in between
    for(uint32_t i = low_index+1; i < high_index; i++) {
      bv->vec[i] |= 0xffffffff;
    }
    mask_low = 0xffffffff - mask_low;
    bv->vec[low_index] |= mask_low;
    bv->vec[high_index] |= mask_high;
  }

  bv->count = high - low + 1;
  return;
}


void mark_all_bv(bv_t *bv)
{
  return _mark_all_bv_by_op(bv, MARK);
}

void unmark_all_bv(bv_t *bv)
{
  return _mark_all_bv_by_op(bv, UNMARK);
}

void _mark_all_bv_by_op(bv_t *bv, bit_op op)
{
  uint32_t bv_size = get_ceil(bv->size, 32);

  switch(op) {
    case MARK:
      for(uint32_t i = 0; i < bv_size; i++) {
        bv->vec[i] |= 0xffffffff;
      }
      bv->count = bv->size;
      break;
    case UNMARK:
      memset(bv->vec, 0, sizeof(int) * bv_size);
      bv->count = 0;
      break;
    default:
      log_err("INVALID BIT OP\n");
    }
}



void destroy_bv(bv_t *bv) {
  if (bv->vec) free(bv->vec);
  if (bv) free(bv);
  return;
}

// helper for debugging
void print_binary(uint32_t n)
{
  while (n) {
    if (n & 1)
        printf("1");
    else
        printf("0");

    n >>= 1;
  }
  printf("\n");
}

