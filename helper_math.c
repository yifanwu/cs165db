#include "helper.h"

/*
 * lower bound algorithm based on binary search
 * get the minimum index that is smaller or equal to the number
 */

uint32_t get_lower_bound(FILE *f, uint32_t size, int num)
{
  if (V) printf("Getting lower bound\n");
  // corner case again!
  if (size == 0) {
    return 0;
  }
  // get first location
  uint32_t low = 0;
  uint32_t high = size;
  uint32_t mid;

  while (low < high) {
    mid = (low + high)/2;
    if (mid == size) {
      // we are done don't read
      return size - 1;
    }
    if (num <= get_val_from_offset(f, mid)) {
      high = mid;
    }
    else {
      low = mid + 1;
    }
  }

  if (low == size) {
    // we are done don't read
    return size - 1;
  }
  if (num <= get_val_from_offset(f, low)) {
    return low;
  }
  else {
    // everything is smaller than num, so insert to the end
    return size;
  }
}

/*
 * upper bound algorithm
 * easier to deal with duplicates
 * get the maximum index that is smaller or equal to the number
 */
uint32_t get_upper_bound(FILE *f, uint32_t size, int num)
{
  if (V) printf("Getting upper bound\n");

  // corner case again!
  if (size == 0) {
    return 0;
  }
  // get first location
  uint32_t low = 0;
  uint32_t high = size;
  uint32_t mid;

  while (low < high) {
    mid = (low + high + 1)/2;

    if (mid == size) {
      return size;
    }
    if (get_val_from_offset(f, mid) <= num) {
      low = mid;
    }
    else {
      high = mid - 1;
    }
  }

  if (get_val_from_offset(f, low) <= num) {
    uint32_t n = size > low + 1? low + 1: size;
    return n;
  }
  else {
    // everything is smaller than num, so insert to the end
    return 0;
  }
}


/**
 * same thing, just takes an array instead
 */

uint32_t get_upper_bound_from_array(int *a, uint32_t size, int num)
{
  // corner case again!
  if (size == 0) {
    return 0;
  }
  // corner case!
  if (size == 1) {
    if (num < a[0]) {
      return 0;
    }
    else {
      return 1;
    }
  }

  // get first location
  uint32_t low = 0;
  uint32_t high = size;
  uint32_t mid;

  while (low < high) {
    mid = (low + high + 1)/2;

    if (a[mid] <= num) {
      low = mid;
    }
    else {
      high = mid - 1;
    }
  }
  if (a[low] <= num) {
    uint32_t n = size > low + 1? low + 1: size;
    return n;
  }
  else {
    // everything is smaller than num, so insert to the end
    return 0;
  }
}

int compare_int (const void * a, const void * b) {
  return ( *(int*)a - *(int*)b );
}


int compare_array (const void * a, const void * b) {
  return ( ((int*)a)[0] - ((int*)b)[0] );
}

int compare_malloced_array (const void * a, const void * b)
{

  const int *l=*(const int**)a;
  const int *r=*(const int**)b;
  return l[0] - r[0];
}

// ceiling of x/y
uint32_t get_ceil(uint32_t x, uint32_t y)
{
  return (x+y-1)/y;
}

// bounded add, not to exceed a number
int bounded_increment(int x, int bound)
{

#ifdef DEBUG
  assert(x <= bound);
#endif
  return x > (bound-1) ? bound : x+1;
}
