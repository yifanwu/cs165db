#include "helper.h"
#include "dbg.h"

/*******************************************
 * LOAD HELPER
 *******************************************/

int load_helper(char *col_names, int **ints_buffer, uint32_t row_size, uint32_t col_size)
{
  // now we actually need to persist the data on disk
  column_t first_c;
  init_col(&first_c, col_names);

  if (first_c.m.type != UNSORTED) {
    // qsort with the tail, UNIT TESTED
    // TODO: external sort, this cannot handle a million rows
    qsort (ints_buffer, row_size, sizeof(ints_buffer[0]), compare_malloced_array);
  }
  debug("qsort succeeded\n");

#ifdef VERBOSE
  log_call("\nQ SORTED result\n");
  for(uint32_t i = 0; i <row_size; i++){
    for (uint32_t j = 0; j < col_size; j ++) {
      debug("%d\t", ints_buffer[i][j]);
    }
  }
#endif
  /********************************
   * SORTED & UNSORTED & BPTREE second index copy
   ********************************/

  // TOOD: change this into block at a time
  // but now I can't be bothered to optimize
  column_t c;
  for (int i = 0; i < col_size; i ++) {
    init_col(&c, &(col_names[i * MAX_NAME_SIZE]));

    debug("LOADING COLUME %s", c.name);

    for (int j = 0; j < row_size; j++) {
      fwrite(&(ints_buffer[j][i]), sizeof(int), 1, c.fp);
    }
    // now update the meta data
    c.m.size = row_size;
    fseek(c.fp, 0, SEEK_SET);
    fwrite(&(c.m), sizeof(column_meta_t), 1, c.fp);
    fclose(c.fp);
    c.fp = NULL;
  }
  /********************************
   * BPTREE tree part
   ********************************/
  if (first_c.m.type == BPTREE) {
    debug("Loading in to BPTree");
    // reconstruct the array
    int *bpt_data = malloc(sizeof(int) * row_size);
    debug("ptr: %p", bpt_data);
    for (int j = 0; j < row_size; j++) {
      // just need to restructure to get the first column
      bpt_data[j] = ints_buffer[j][0];
    }
    bulk_load(first_c.bpt_fp, bpt_data, row_size);

#ifdef VERBOSE
    print_bpt(first_c.bpt_fp);
#endif
    free(bpt_data);
    fclose(first_c.bpt_fp);
    first_c.bpt_fp = NULL;
  }
  return 0;
}

