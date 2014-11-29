#include "helper.h"
#include "dbg.h"

/**********************************************************************
 * CREATE
 * for UNSORTED, SORTED, and BPTREE
 **********************************************************************/
void make_new_column_file(char *name, storage_t type)
{

  if(V) printf("MAKE NEW FILE:\t%s of type %d\n", name, type);
  FILE *f = get_file_for_make(name);

  // must allocate space otherwise fwrite doesn't work
  column_meta_t *m = malloc(sizeof(column_meta_t));
  m->type = type;
  m->size = 0;
  m->next_free = 0;
  m->root = 0;


  if (fwrite(m, sizeof(column_meta_t), 1, f) != 1) {
    goto error;
  }

  if (type == BPTREE) {

    // need to create the other file!
    // note that the column meta data in the other file is
    // just a place holder and *not* updated
    char *file_name_ext = get_bpt_nodes_file_name(name);
    FILE *bpt_f = fopen(file_name_ext, "wb+");

    if (fwrite(m, sizeof(column_meta_t), 1, bpt_f) != 1) {
      goto error;
    }

    free(file_name_ext);
    fclose(bpt_f);
  }

  fclose(f);

  free(m);
  return;

error:
  perror("WRITE");
  exit(EXIT_FAILURE);
}

char *get_file_name(char *name)
{
  char *file_name_ext = malloc(strlen(name)+strlen(DATA_EXTENTION) + 2);
  sprintf(file_name_ext, "%s.%s", name, DATA_EXTENTION);
  verbose("Working with file: %s\n", file_name_ext);
  return file_name_ext;
}


char *get_bpt_nodes_file_name(char *name)
{
  char *file_name_ext = malloc(strlen(name)+strlen(BPT_RAW) + strlen(DATA_EXTENTION) + 3);
  sprintf(file_name_ext, "%s_%s.%s", name, BPT_RAW, DATA_EXTENTION);
  return file_name_ext;
}

FILE *_get_file(char *name, const char *op)
{
  dbg_assert(strlen(name) != 0);
  char *file_name = get_file_name(name);
  // open file for read and write
  FILE *f = fopen(file_name, op);
  free(file_name);
  check(f,"reading file %s", file_name);
  return f;
error:
  exit(EXIT_FAILURE);
}

FILE *get_file_for_read(char *name)
{
  return _get_file(name, "r");
}


FILE *get_file_for_update(char *name)
{
  return _get_file(name, "rb+");
}

FILE *get_file_for_make(char *name)
{
  return _get_file(name, "w+");
}
