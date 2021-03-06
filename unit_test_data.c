#include "unit_test.h"

void ut_small_bpt() {
  printf("Testing small b+ tree.\n\n");
  char *name = "test_bpt";
  make_new_column_file(name, BPTREE);

  column_t *col = malloc(sizeof(column_t));
  strcpy(col->name, name);
  col->m.size = 0;
  col->m.next_free = 0;
  col->m.root = 0;
  col->bpt_fp = NULL;
  col->bpt_fp = NULL;
  init_col(col, name);

  bpt_insert_record(col->bpt_fp, 10, 0);
  bpt_insert_record(col->bpt_fp, 1, 1);
  bpt_insert_record(col->bpt_fp, 5, 2);
  bpt_insert_record(col->bpt_fp, 6, 3);
  fflush(col->bpt_fp);
#ifdef UT_VERBOSE
  print_bpt(col->bpt_fp);
#endif
  uint32_t *bv = create_bv(4);
  find_record_in_range(col->bpt_fp, bv, 5,5);
  assert(is_marked(bv, 2));
  return;
}

void ut_bpt() {
  printf("Testing b+ tree.\n\n");
  char *name = "test_bpt";
  make_new_column_file(name, BPTREE);

  column_t *col = malloc(sizeof(column_t));
  init_col(col, name);

  // insert
  bpt_insert_record(col->bpt_fp, 10, 0);
  bpt_insert_record(col->bpt_fp, 1, 1);
  bpt_insert_record(col->bpt_fp, 5, 2);
  bpt_insert_record(col->bpt_fp, 6, 3);
  bpt_insert_record(col->bpt_fp, 12, 4);
  bpt_insert_record(col->bpt_fp, 11, 5);
  bpt_insert_record(col->bpt_fp, 15, 6);
  bpt_insert_record(col->bpt_fp, 2, 7);

#ifdef UT_VERBOSE
  print_bpt(col->bpt_fp);
#endif

  uint32_t *bv = create_bv(8);
  find_record_in_range(col->bpt_fp, bv, 5,5);
  assert(is_marked(bv, 2));
  find_record_in_range(col->bpt_fp, bv, 10,10);
  assert(is_marked(bv, 0));
  unmark_all_bv(bv,8);
  find_record_in_range(col->bpt_fp, bv, 1,1);
  assert(is_marked(bv, 1));
  unmark_all_bv(bv,8);
  find_record_in_range(col->bpt_fp, bv, 0,8);
  assert(is_marked(bv, 1));
  assert(is_marked(bv, 2));
  unmark_all_bv(bv,8);
  find_record_in_range(col->bpt_fp, bv, 2,2);
  assert(is_marked(bv, 7));
  unmark_all_bv(bv,8);
  find_record_in_range(col->bpt_fp, bv, 15,15);
  assert(is_marked(bv, 6));
  unmark_all_bv(bv,8);
  find_record_in_range(col->bpt_fp, bv, 0,25);
  assert(is_marked(bv, 0));
  assert(is_marked(bv, 1));
  assert(is_marked(bv, 2));
  assert(is_marked(bv, 3));
  assert(is_marked(bv, 4));
  assert(is_marked(bv, 5));
  assert(is_marked(bv, 6));
  assert(is_marked(bv, 7));
  unmark_all_bv(bv,8);

  printf("SUCCESS!\n");
  return;
}

