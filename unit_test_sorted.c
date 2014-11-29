#include "unit_test.h"


void ut_sorted_bounds(int size)
{
  printf("Testing SORTED with BOUNDS.\n\n");
  char *name = "test_sorted_bounds";
  make_new_column_file(name, SORTED);
  column_t *c = malloc(sizeof(column_t));
  init_col(c, name);
  bv_t *bv = create_bv(size);

  int insert_index;

  for (int i = size-1; i>=0; i--) {
    insert_index = get_lower_bound(c->fp, c->m.size, i);
    insert_value_to_file(c, insert_index, i);
  }

  printf("Finished inserting data.\n");

#ifdef UT_VERBOSE
  print_data_file(c->bpt_fp);
#endif
  for (int i = 0; i < size; i++) {
    printf("Searching values at %d\n", i);
    mark_matching_bv_for_sorted(c, bv, i, i);
    assert(is_marked(bv, i));
    unmark_all_bv(bv);
  }
  return;
}


void ut_sorted()
{

  printf("Testing SORTED.\n\n");
  char *name = "test_sorted";
  make_new_column_file(name, SORTED);
  column_t *c = malloc(sizeof(column_t));
  init_col(c, name);

  // let's load!
  insert_value_to_file(c, 0, 5);
  insert_value_to_file(c, 0, 1);
  insert_value_to_file(c, 1, 2);
  insert_value_to_file(c, 2, 3);
  // final: 1 2 3 5

  printf("Data inserted\n");
  int num;
  int count;
  fseek(c->fp, 0, SEEK_SET);
  column_meta_t m;
  fread(&m, sizeof(column_meta_t), 1, c->fp);
  printf("Size written in file is %d\n", m.size);
  assert(m.size == 4);
  count = fread(&num, sizeof(int), 1, c->fp);
  printf("Checking values at 0, we got %d\n", num);
  assert(count == 1);
  assert(num == 1);
  printf("Checking values at 1\n");
  count = fread(&num, sizeof(int), 1, c->fp);
  assert(count == 1);
  assert(num == 2);
  printf("Checking values at 2\n");
  count = fread(&num, sizeof(int), 1, c->fp);
  assert(count == 1);
  assert(num == 3);
  printf("Checking values at 3\n");
  count = fread(&num, sizeof(int), 1, c->fp);
  assert(count == 1);
  assert(num == 5);

  bv_t *bv = create_bv(4);

  printf("Searching values at 1\n");
  mark_matching_bv_for_sorted(c, bv, 1, 1);
  assert(is_marked(bv, 0));
  assert(!is_marked(bv, 1));
  unmark_all_bv(bv);

  printf("Searching values at 2\n");
  mark_matching_bv_for_sorted(c, bv, 2, 2);
  assert(is_marked(bv, 1));
  assert(!is_marked(bv, 0));
  unmark_all_bv(bv);

  printf("Searching values at 3\n");
  mark_matching_bv_for_sorted(c, bv, 3, 3);
  assert(is_marked(bv, 2));
  assert(!is_marked(bv, 1));
  unmark_all_bv(bv);

  printf("Searching values at 5\n");
  mark_matching_bv_for_sorted(c, bv, 5, 5);
  assert(is_marked(bv, 3));
  assert(!is_marked(bv, 1));
  unmark_all_bv(bv);

  printf("Searching values between 1 and 3\n");
  mark_matching_bv_for_sorted(c, bv, 1, 3);
  assert(is_marked(bv, 0));
  assert(is_marked(bv, 1));
  assert(is_marked(bv, 2));
  assert(!is_marked(bv, 3));
  unmark_all_bv(bv);

  return;
}


void ut_unsorted(int size)
{

  printf("Testing UNSORTED.\n\n");
  char *name = "test_unsorted";
  make_new_column_file(name, SORTED);
  column_t *c = malloc(sizeof(column_t));
  init_col(c, name);

  // append things!
  for(int i = 0; i < size; i ++) {
    append_value_to_file(c, i);
    append_value_to_file(c, i-1);
  }

  print_data_file(c->fp);
  init_col(c, name);
  bv_t *bv = create_bv(2 * size);

  for(int i = 0; i < size - 3; i ++) {
    mark_matching_bv_for_unsorted(c, bv, i-1, i);
    assert(is_marked(bv, (2*i)));
    assert(is_marked(bv, (2*i + 1)));
    assert(is_marked(bv, (2*i + 3)));
    unmark_all_bv(bv);
  }
  return;
}


