#include "helper.h"
#include "dbg.h"

// help get a list of positions from variable
int get_pos_from_var(list_t *vars, char *var_name, int **pos)
{
  variable_t *pos_var = find_variable(vars, var_name);
  int num_pos = 0;
  if (pos_var->is_id) {
    // dealing with row id, assign directly
    num_pos = get_list_size(pos_var->ids);
    *pos = calloc(num_pos, sizeof(int));
    memcpy(*pos, pos_var->ids->data, sizeof(int) * num_pos);
  } else {
    // now we need to convert from bit vector
    num_pos = get_bv_count(pos_var->bv);
    *pos = calloc(num_pos, sizeof(int));
    // go through the whole bit vector to get the values
    // TODO: should be a better strategy than this!
    uint32_t size = get_bv_size(pos_var->bv);
    uint32_t itr = 0;
    for (uint32_t u=0; u < size; u++) {
      if (is_marked(pos_var->bv, u)) {
        (*pos)[itr] = u;
        itr ++;
      }
    }
    debug("Itr ended at %d\n", itr);
    dbg_assert(itr == num_pos);
  }

  return num_pos;
}


  /*******************************************************************
   * DELETE HELPER
   *
   * overwrite the last value one with the position
   * it is assumed to be consistent since this is done for all columns
   * note that if a crash happened this would be left in an inconsistent state
   *
   * might delete multiple at the same time
 *******************************************************************/

int delete_helper(char *col_name, int *delete_pos, uint32_t num_pos)
{
  column_t c;
  init_col(&c, col_name);
  dbg_assert(c.m.type == UNSORTED);

  int last_val;
  uint32_t num_read;
  for (uint32_t u = 0; u < num_pos; u++) {
    fseek(c.fp, sizeof(column_meta_t) + (c.m.size - 1) * sizeof(int), SEEK_SET);
    num_read = fread(&last_val, sizeof(int), 1, c.fp);
    dbg_assert(num_read == 1);
    debug("We are deleting position %d with position %d\n", delete_pos[u], c.m.size - 1);
    fseek(c.fp, sizeof(column_meta_t) + delete_pos[u] * sizeof(int), SEEK_SET);
    fwrite(&last_val, sizeof(int), 1, c.fp);
    // won't actually delete the data until this gets overwritten by the next value
    // saves time moving everything around
    c.m.size--;
  }

  fseek(c.fp, 0, SEEK_SET);
  fwrite(&(c.m), sizeof(column_meta_t), 1, c.fp);
  // force flush and release space
  fclose(c.fp);
  return 0;
}

/*******************************************************************
 * UPDATE HELPER
 *
 * very similar to delte
 * just don't need to do the swapping but just updating in place
 *******************************************************************/

int update_helper(char *col_name, int *update_pos, uint32_t num_pos, int val)
{
  column_t c;
  init_col(&c, col_name);
  dbg_assert(c.m.type == UNSORTED);

  for (uint32_t u = 0; u < num_pos; u++) {
    fseek(c.fp, sizeof(column_meta_t) + update_pos[u] * sizeof(int), SEEK_SET);
    fwrite(&val, sizeof(int), 1, c.fp);
  }
  // force flush and release space
  fclose(c.fp);
  return 0;
}
