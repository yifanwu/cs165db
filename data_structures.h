#ifndef DATA_STRUCTURES_H_
#define DATA_STRUCTURES_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdint.h>
#include <regex.h>
#include "config.h"
#include "threadpool.h"
#include "bit_vector.h"
#include "list.h"

typedef enum {UNSORTED, SORTED, BPTREE} storage_t;
typedef enum {IN_VAR, IN_COMMAND, IN_ARG_ONE, IN_ARG_TWO, IN_ARG_THREE} input_t;
//typedef enum {ADD, SUB, DIV, MUL} math_op;
//typedef enum {MAX, MIN, AVG} aggr_op;

typedef struct
{
  uint32_t expected_bytes;
} socket_meta_t;

typedef struct {
  storage_t type;
  uint32_t size;
  // these two are just for btree but it's kept
  // here since it's eaiser to deal with
  uint32_t next_free;
  uint32_t root;
} column_meta_t;

typedef struct {
  char name[MAX_NAME_SIZE];
  column_meta_t m;
  FILE *fp;
  FILE *bpt_fp;
} column_t;

typedef struct {
  char name[MAX_NAME_SIZE];
  bool is_id;
  bool is_fetched;
  bool is_materialized;
  list_t *ids; // could be either IDs
  bv_t *bv; // or bit vectors
  column_t fetched_col;
  uint32_t val_size;
  int *val;
} variable_t;

typedef struct {
  int end;
  int start;
} search_result_t;

typedef struct {
  regex_t *r;
  thread_pool_t *pool;
} thread_arg;

#endif //DATA_STRUCTURES_H_
