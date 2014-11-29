#include "helper.h"
/**********************************************************************
 *
 * MISC
 **********************************************************************/

bool is_empty_str(char *in_str)
{
  if (in_str == NULL || strlen(in_str) == 0) {
    return true;
  } else {
    return false;
  }
}

/**
 * holder functions for GDB
 **/

void gdb_break(void)
{
  return;
}

void print_data_file(FILE *f)
{
  int num;
  column_meta_t m;
  fseek(f, 0, SEEK_SET);
  fread(&m, sizeof(column_meta_t), 1, f);
  printf("File size: %d\n",  m.size);
  for (uint32_t i = 0; i < m.size; i ++) {
    fread(&num, sizeof(int), 1, f);
    printf("%d\t", num);
  }
  fclose(f);
  return;
}


void print_data_file_from_ptr(FILE *f)
{
  int num;
  column_meta_t m;
  fseek(f, 0, SEEK_SET);
  fread(&m, sizeof(column_meta_t), 1, f);
  printf("File size: %d\n", m.size);
  for (uint32_t i = 0; i < m.size; i ++) {
    fread(&num, sizeof(int), 1, f);
    printf("%d\t", num);
  }
  return;
}
