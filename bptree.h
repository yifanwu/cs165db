#ifndef BPTREE_H_
#define BPTREE_H_
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <math.h>
#include <assert.h>
#include "config.h"
#include "bit_vector.h"
#include "helper.h"
#include "data_structures.h"


typedef struct {
  int keys[BPT_SIZE - 1];
  uint32_t pointers[BPT_SIZE]; // these point to file offsets
  bool is_leaf;
  uint32_t parent;
  uint32_t size;
  uint32_t next;
  uint32_t pos;
} node_t;

// exposed functions
void bpt_insert_record(FILE *f, int key, uint32_t row_index);
uint32_t _find_record_in_range_type(FILE *f, bv_t *bv, list_t *id_list, bool is_id, int low, int high);
uint32_t find_record_in_range(FILE *f, bv_t *bv, int low, int high);
uint32_t find_record_in_range_id(FILE *f, list_t *ids, int low, int high);
int bulk_load(FILE *f, int *sorted, uint32_t size);

// intermediates
node_t *_find_leaf(FILE *f, node_t *root, int key);
void _insert_node(FILE *f, column_meta_t *cm, node_t *n, int key, uint32_t pointer);

// node_t *_leaf_insert(FILE *f, column_meta_t *cm, node_t *leaf, int key, uint32_t pointer, node_t *root);
// node_t *_node_insert(FILE *f, column_meta_t *cm, node_t *root, node_t *node, int new_key, node_t *new_node);

// helpers

node_t *_make_new_leaf(FILE *f, column_meta_t *cm);
node_t *_make_new_node(FILE *f, column_meta_t *cm);
node_t *_make_new_leaf_or_node(FILE *f, column_meta_t *cm, bool is_leaf);

// void _load_in_temp(int *temp_keys, uint32_t *temp_pointers, int index, int new_key, node_t *node, node_t *new_node);
// void _split_temp_leaf(FILE *f, int *temp_keys, uint32_t *temp_pointers, int split_key, node_t *node, node_t *new_node);
// void _split_temp_node(FILE *f, int *temp_keys, uint32_t *temp_pointers, node_t *node, node_t *new_node);
void _add_entry_to_node(FILE *f, node_t *node, int key, uint32_t pointer);
void _load_node(FILE *f, node_t *node, uint32_t pos);
void _write_node(FILE *f, node_t *node);
void _node_io(FILE *f, node_t *node, uint32_t pos,  bool is_read);
void _write_cm(FILE *f, column_meta_t *cm);
void print_bpt(FILE *f);
void print_bpt_node(node_t *holder);
size_t _get_bpt_file_cursor(uint32_t num);

#endif
