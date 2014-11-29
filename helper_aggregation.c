#include "helper.h"
#include "dbg.h"

int aggregate_vals(variable_t *v, int (*op)(int,int), bool is_avg, int *result_holder)
{
  log_call("Aggregating Data");

  // check what the variable is identified by

  int *materialized_result = NULL;
  uint32_t materialized_size = 0;
  if (v->is_materialized) {
    materialized_result = v->val;
    materialized_size = v->val_size;
    // TODO: delete
    //for(uint32_t u = 0; u < materialized_size; u++) {
    //  debug("val is %d\n", v->val[u]);
    //}
  } else {
    materialized_result = fetch_matched_from_bv(&(v->fetched_col), v->bv);
    materialized_size = get_bv_count(v->bv);
  }

  // getting the first value to get the iteration started
  int val = materialized_result[0];
  for(uint32_t i = 1; i < materialized_size; i++) {
    val = op(val, materialized_result[i]);
  }

  if (is_avg) {
    debug("deviding is %d by %d", val, materialized_size);
    assert(materialized_size != 0);
    val /= materialized_size;
  }

  *result_holder = val;
  debug("AGGREGATE RESULT IS: %d", val);
  free(materialized_result);
  return 0;
}


int fptr_max(int a, int b)
{
  if (a > b) return a;
  else return b;
}

int fptr_min(int a, int b)
{
  if (a < b) return a;
  else return b;
}

int fptr_inc(int a, int b)
{
  // keeping the b here just for sake of functional overloading
  return a++;
}


