#include "helper.h"

int get_val_from_offset(FILE *f, uint32_t offset)
{
  int val;
  int count;
  fseek(f, sizeof(column_meta_t) + sizeof(int) * offset, SEEK_SET);
  count = fread(&val, sizeof(int), 1, f);

  if (count != 1) {
    perror("offset wrong");
    exit(EXIT_FAILURE);
  }
  return val;
}


