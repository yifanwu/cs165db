#include "helper.h"
#include "dbg.h"


/*******************************************************************
 * setting up new variable
 *
 * a variable is very over loaded
 * - could be identitified by a bitvector
 * - row_id (allows for repeats in joins)
 * - or just materialized already
 *
 * we also allow for reuse of variable names
 * - need to find if the variable exists already, if so destructively update...
 *******************************************************************/

variable_t *_initialize_variable_types(list_t *vars, char *name, bool is_materialized, uint32_t size, bool is_id)
{
  debug("Initializing Variable");

  // check if variable exists already!
  variable_t *f_var = find_variable(vars, name);
  if (f_var != NULL) return f_var;

  variable_t new_var;
  strcpy(new_var.name, name);
  if (is_materialized == false || is_id == false) {
    new_var.bv = create_bv(size);
  } else {
    new_var.bv = NULL;
  }
  if (is_id) {
    // make the list of id
    new_var.ids = create_list(sizeof(uint32_t));
  }
  new_var.is_fetched = false;
  new_var.is_id = is_id;
  new_var.is_materialized = is_materialized;
  new_var.val = NULL;
  new_var.val_size = 0;
  variable_t *ptr = (variable_t *)append_list(vars, &new_var);
  return ptr;
}

variable_t *initialize_variable(list_t *vars, char *name, uint32_t size)
{
  return _initialize_variable_types(vars, name, false, size, false);
}

variable_t *initialize_variable_materialized(list_t *vars, char *name)
{
  return _initialize_variable_types(vars, name, true, 0, false);
}

variable_t *initialize_rowid_variable(list_t *vars, char *name)
{
  return _initialize_variable_types(vars, name, false, 0, true);
}




/*******************************************************************
 * SELECT HELPER
 *******************************************************************/

/* UNSORTED select
 * note that this is overloaded for equal */

uint32_t mark_matching_bv_for_unsorted(column_t *c, bv_t *bv, int low, int high)
{
  debug("SELECT: MARKING BV FOR UNSROTED");
  uint32_t total = 0;
  uint32_t read;
  int* buffer;
  // don't forget to reset the pointer!
  fseek(c->fp, sizeof(column_meta_t), SEEK_SET);

#ifdef NOBLOCKREAD

  buffer = (int*) malloc(sizeof(int) * c->m.size);
  check_mem(buffer);
  // just read everything
  read = fread(buffer, sizeof(int), c->m.size, c->fp);
  if (read != c->m.size) {
    printf("Didn't read everything!\n");
    assert(false);
  }

  for (uint32_t j = 0; j < c->m.size; j++) {
    if ((buffer[j] >= low) && (buffer[j] <= high)) {
      mark_bv(bv, j);
      total++;
    }
  }

#else
  uint32_t index = 0;

  // read blcok at a time
  buffer = (int*) malloc(sizeof(int) * FILE_BLOCK_SIZE);
  check_mem(buffer);
  uint32_t block_count = get_ceil(c->m.size, FILE_BLOCK_SIZE);

  for(uint32_t u = 0; u < block_count; u++) {
    read = fread(buffer, sizeof(int), FILE_BLOCK_SIZE, c->fp);
    if (read == 0) {
      log_err("mark cannot read column");
      return 0;
    }
    for (uint32_t j = 0; j < read; j++) {
      index = u * FILE_BLOCK_SIZE + j;
      if ((buffer[j] >= low) && (buffer[j] <= high)) {
        mark_bv(bv, index);
        total++;
      }
    }
  }

#endif

  free(buffer);

  return total;

error:
  return 0;
}

/* SORTED select
 * overloaded for equal */

uint32_t mark_matching_bv_for_sorted(column_t *c, bv_t *bv, int low, int high)
{
  uint32_t low_index = get_lower_bound(c->fp, c->m.size, low);
  uint32_t high_index = get_upper_bound(c->fp, c->m.size, high);
  high_index = high_index > 0 ? high_index - 1: 0;
  assert(low <= high);

  if(V) printf("marking range, %d, %d\n", low_index, high_index);
  if (low_index == high_index) {
    mark_bv(bv, low_index);
  } else {
    mark_bv_by_range(bv, low_index, high_index);
  }

  return (high_index - low_index + 1);
}

/*******************************************************************
 * FETCH HELPER
 *******************************************************************/

variable_t *find_variable(list_t *vars, char *name)
{
  assert(strlen(name) > 0);
  uint32_t size = get_list_size(vars);
  for (uint32_t i = 0;i < size; i ++) {
    // get the pointer to the var
    variable_t *v = (variable_t *)get_list_val(vars, i);
    if (strcmp(name,v->name) == 0) {
      // found and return; assume no duplicates (do not support duplicates)
      return v;
    }
  }
  // nothing found
  return NULL;
}



