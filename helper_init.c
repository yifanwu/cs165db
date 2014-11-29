#include "helper.h"
#include "dbg.h"

// assumes that the data is allocated already
int init_col(column_t *c, char *name)
{
  // initialize so no junk values, just to be safe
  c->m.type = UNSORTED;
  c->m.size = 0;
  c->m.next_free = 0;
  c->m.root = 0;
  c->fp = NULL;
  c->bpt_fp = NULL;
  memset(c->name,'\0', MAX_NAME_SIZE);
  strcpy(c->name, name);
  c->fp = get_file_for_update(name);
  fseek(c->fp, 0, SEEK_SET); // just to be sure
  if (fread(&c->m, sizeof(column_meta_t), 1, c->fp) != 1) {
    log_err("data corrupt for column %s \n", name);
  }
  if (c->m.type == BPTREE) {
    // get the other file
    char *file_name_ext = get_bpt_nodes_file_name(name);
    c->bpt_fp = fopen(file_name_ext, "rb+");
    free(file_name_ext);
    check_mem(c->bpt_fp);

  }
  return 0;

error:
  return 1;
}

int load_col(column_t *c)
{
  c->fp = get_file_for_update(c->name);
  fseek(c->fp, 0, SEEK_SET); // just to be sure
  if (fread(&c->m, sizeof(column_meta_t), 1, c->fp) != 1) {
    log_err("data corrupt for column %s \n", c->name);
  }
  if (c->m.type == BPTREE) {
    // get the other file
    char *file_name_ext = get_bpt_nodes_file_name(c->name);
    c->bpt_fp = fopen(file_name_ext, "rb+");
    free(file_name_ext);
    check_mem(c->bpt_fp);
  }
  return 0;

error:
  return 1;
}

void close_col_files(column_t *c)
{
  if (!c->fp) fclose(c->fp);
  if (!c->bpt_fp) fclose(c->bpt_fp);
  return;
}
