#include "helper.h"
#include "list.h"
#include "dbg.h"

/****************************************************************
 * FETCH from ROWID
 *
 * we want to sort the list before we fetch
 *
 * will implement clustering sort, but for now just qsort
 * // sort it!
 * qsort(v->ids->data, get_list_size(v->ids), sizeof(int), compare_int);
 *
 ****************************************************************/
int *fetch_matched_from_rowid(column_t *c, variable_t *v)
{
  debug("fetching from row id!");
  size_t read;
  // already loaded the meta data before fetch is called
  // I'm just going to load everything in for now...
  uint32_t id_size = get_list_size(v->ids);
  int *buffer = malloc(sizeof(int) * c->m.size);
  int *result = malloc(sizeof(int) * id_size);
  // now go and get them
  fseek(c->fp, sizeof(column_meta_t), SEEK_SET);
  read = fread(buffer, sizeof(int), c->m.size, c->fp);
#ifdef DEBUG
  if (read != c->m.size) {
    log_err("Didn't read everything in for the fetch!");
  }
  // print the file
  //print_data_file_from_ptr(c->fp);
#endif
  for (uint32_t i= 0; i < id_size; i ++) {
    result[i] = buffer[*(int *)get_list_val(v->ids,i)];
  }

  free(buffer);

  return result;
}

/****************************************************************
 * FETCH from bit vector
 *
 ****************************************************************/
int *fetch_matched_from_bv(column_t *c, bv_t *bv)
{
  log_call("MATERIALIZING the bit vector");
  uint32_t u;
  uint32_t *buffer = calloc(FILE_BLOCK_SIZE, sizeof(int));
  uint32_t block_count = get_ceil(c->m.size, FILE_BLOCK_SIZE);
  uint32_t read;
  uint32_t index = 0;
  uint32_t matched_total = get_bv_count(bv);
  int *result = calloc(matched_total, sizeof(int));
  int matched_counter = 0;


  debug("We are about to read %d num of blocks for %d num of data\n", block_count, c->m.size);

  // just to be sure
  fseek(c->fp, sizeof(column_meta_t), SEEK_SET);

  for(u = 0; u < block_count; u++) {
    read = fread(buffer, sizeof(int), FILE_BLOCK_SIZE, c->fp);
    if (read == 0) {
      log_err("Fetch cannot read column");
      return NULL;
    }
    for (uint32_t k = 0; k < read; k++) {
      index = u * FILE_BLOCK_SIZE + k;
      if (is_marked(bv, index) == true) {
        result[matched_counter] = buffer[k];
        matched_counter ++;
      }
    }
  }
  free(buffer);
  debug("Finished materializing the bitvector");
  return result;
}

/*******************************************************************************
 * TUPL HELPER
 * We are actually leaving the tuple reconstruction to the client side
 * In the sense that data here is still grouped by
 *******************************************************************************/
int *tuple_helper(list_t *vars, char *data_buffer, int v_count, list_t *result, uint32_t *result_size)
{

  debug("TUPLE HELPER");

  variable_t *tup_vars[v_count];
  // get the first set
  tup_vars[0] = find_variable(vars, data_buffer);

  /****************************************
   * we are using values such as add sub etc.
   * it's materialized
   * TODO: be able to support a mix of different types!
   ***************************************/
  if (tup_vars[0]->is_materialized ) {

    uint32_t row_num = tup_vars[0]->val_size;

#ifdef DEBUG
    assert(row_num > 0);
#endif

    *result_size = row_num * v_count;
    int *tup_res = (int *)malloc(sizeof(int) * (*result_size));
    for (uint32_t i = 0; i < v_count; i++) {
      tup_vars[i] = find_variable(vars, &(data_buffer[i * MAX_NAME_SIZE]));
#ifdef DEBUG
      assert(tup_vars[i]->val_size == row_num);
      assert(tup_vars[i]->val != NULL);
#endif
      memcpy(&(tup_res[i * row_num]), tup_vars[i]->val, row_num * sizeof(int));
    }
    debug("Tuple Heler Finished");
    return tup_res;
  }

  /****************************************
   * Make from file
   *
   ***************************************/
#ifdef DEBUG
  assert(tup_vars[0]->is_fetched == true);
#endif
  load_col(&(tup_vars[0]->fetched_col));
  uint32_t max_row = tup_vars[0]->fetched_col.m.size;
  assert(max_row > 0);
  int *tuple_buffer = malloc(sizeof(int) * max_row);
  check_mem(tuple_buffer);

  debug("Now processing the info for %d num of variables of size %d.\n", v_count, max_row);

  for (int i = 0; i < v_count; i++) {
    tup_vars[i] = find_variable(vars, &(data_buffer[i * MAX_NAME_SIZE]));

#ifdef DEBUG
    assert(tup_vars[i]);
    assert(tup_vars[i]->is_fetched == true);
    assert(tup_vars[i]->fetched_col.name != NULL);
#endif
    load_col(&(tup_vars[i]->fetched_col));

    fseek(tup_vars[i]->fetched_col.fp, sizeof(column_meta_t), SEEK_SET);
    size_t read = fread(tuple_buffer, sizeof(int), max_row, tup_vars[i]->fetched_col.fp);
    if (read != max_row) {
      log_err("Cannot seem to read them all at once! Only had %u (needed %d), need to fix\n", (uint32_t)read, max_row);
    }
    for (uint32_t u = 0; u < max_row; u++) {
      if (is_marked(tup_vars[i]->bv, u)) {
        append_list(result, &tuple_buffer[u]);
      }
    }
    // we want to close all the fd opened
    fclose(tup_vars[i]->fetched_col.fp);
    tup_vars[i]->fetched_col.fp = NULL;
  }

  if(tuple_buffer) free(tuple_buffer);
  *result_size = result->next;
  return (int *)result->data;
error:
  return NULL;
}
