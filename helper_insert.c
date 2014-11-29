#include "helper.h"
#include "dbg.h"

/*******************************************************************
 * INSERT HELPER
 *******************************************************************/

int insert_helper(char *col_name, int num_insert)
{

  // create the column to be worked on
  column_t c;
  init_col(&c, col_name);

  if (c.m.type == UNSORTED) {
    append_value_to_file(&c, num_insert);
  }
  else if (c.m.type == SORTED) {
    // find the location to inser
    int insert_index = get_lower_bound(c.fp, c.m.size, num_insert);
    insert_value_to_file(&c, insert_index, num_insert);
  }
  else {
    assert(c.m.type == BPTREE);
    bpt_insert_record(c.bpt_fp, num_insert, c.m.size);
    // I'm also keeping another file that could be directly index by location
    append_value_to_file(&c, num_insert);

    fclose(c.bpt_fp);
    c.bpt_fp = NULL;
  }

  fclose(c.fp);
  return 0;
}


/********************************
 * writing result of INSERT
 * for UNSORTED
 ********************************/

int append_value_to_file(column_t *col, int val)
{
  column_meta_t m;

  fseek(col->fp, 0, SEEK_SET);

  if (fread(&m, sizeof(column_meta_t), 1, col->fp) != 1) {
    goto error;
  }

  m.size ++;
  fseek(col->fp, 0, SEEK_SET);

  if (fwrite(&m, sizeof(column_meta_t), 1, col->fp) != 1) {
    goto error;
  }

  fseek(col->fp, 0, SEEK_END);

  if (fwrite(&val, sizeof(int), 1, col->fp) != 1) {
    goto error;
  }

  fseek(col->fp, 0, SEEK_SET);

  if (fwrite(&m, sizeof(column_meta_t), 1, col->fp) != 1) {
    goto error;
  }

  return 0;
error:
  perror("error writing to file");
  exit(EXIT_FAILURE);
}



/********************************
 * writing result of INSERT
 * for SORTED
 * should work even with size 0 and index 0
 *******************************/

void insert_value_to_file(column_t *col, int index, int val)
{
  verbose("INSERT to FILE for sorted\n");
  // create a temporary file
  char *temp_name = "temp";
  FILE *new_fp = fopen(temp_name, "w+");

  fseek(new_fp, sizeof(column_meta_t), SEEK_SET);
  fseek(col->fp, sizeof(column_meta_t), SEEK_SET);

  char *file_name = get_file_name(col->name);

  // TOOD: make sure stack is big enough!
  int *first = malloc(sizeof(int) * index);

  // the size is not yet imcremented!
  int rest_num = col->m.size - index;
  printf("Inserting at index %d, current size is %d, \n", index, col->m.size);
  assert(rest_num >= 0);
  int *last = malloc(sizeof(int) * rest_num);

  if (index > 0) {
    if (fread(first, sizeof(int), index, col->fp) != (uint32_t)index) {
      goto error;
    }

    // now right this in
     if (fwrite(first, sizeof(int), index, new_fp) != (uint32_t)index) {
      goto error;
    }
  }

  // write the new thing in
  if (fwrite(&val, sizeof(int), 1, new_fp) != 1) {
    goto error;
  }

  // write the rest in
  if (fread(last, sizeof(int), rest_num, col->fp) != (uint32_t)rest_num) {
    goto error;
  }

  // now right this in
   if (fwrite(last, sizeof(int), rest_num, new_fp) != (uint32_t)rest_num) {
    goto error;
  }

  // update the file size
  fseek(new_fp, 0, SEEK_SET);

  col->m.size ++;
  if (fwrite(&(col->m), sizeof(column_meta_t), 1, new_fp) != 1) {
    goto error;
  }

  col->fp = new_fp;
  // remove old data
  //TODO: need to ensure that this is atomic!
  remove(file_name);
  rename(temp_name, file_name );
  free(file_name);
  free(first);
  free(last);
  return;

error:
  free(file_name);
  perror("error writing to file");
  exit(EXIT_FAILURE);
}


