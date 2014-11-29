#include "bptree.h"
#include "dbg.h"

/*********************************************************************************
 * Yifan's B Plus Tree implementation
 *
 * with help from OH and some online resources
 *      for the overflow design
 * chose to use parent instead of pure recursion
 *      for clarity
 *********************************************************************************/

/*********************************************************************************
 * find record from key
 *
 * public function for finding the record associated with a key
 *********************************************************************************/

// returns an array of keys and their records and count
uint32_t _find_record_in_range_type(FILE *f, bv_t *bv, list_t *id_list, bool is_id, int low, int high)
{
  debug("BPTREE FIND for %d and %d\n", low, high);
  column_meta_t *cm = malloc(sizeof(column_meta_t));
  fseek(f, 0, SEEK_SET);
  fread(cm, sizeof(column_meta_t), 1, f);
  // load root, it's always the first element
  node_t *root = malloc(sizeof(node_t));
  _load_node(f, root, cm->root);

  // first find corresponding leaf
  node_t *low_leaf = _find_leaf(f, root, low);
  if (low_leaf == NULL) {return 0;}

  uint32_t count = 0;
  // must copy memory over otherwise it's junk value!
  // TODO: this is redundant!
  node_t leaf_itr;
  memcpy(&leaf_itr, low_leaf, sizeof(node_t));
  int next_pos = 0;
  do
  {
    for (uint32_t i = 0; i < leaf_itr.size; i++) {
      if (leaf_itr.keys[i] <= high && leaf_itr.keys[i] >= low ) {
        verbose("We found a matching node at %d\n", leaf_itr.pointers[i]);
        if (is_id) {
          append_list(id_list, &(leaf_itr.pointers[i]));
        } else {
          mark_bv(bv, leaf_itr.pointers[i]);
        }
        count ++;
      }
      if (leaf_itr.keys[i] > high) {
        goto done;
      }
    }
    // we are not done yet, need to go down in case of overflow for duplicates
    next_pos = leaf_itr.next;
    _load_node(f, &leaf_itr, leaf_itr.next);
  } while (next_pos != 0);

done:
  if(root) free(root);
  if(cm) free(cm);
  if(low_leaf) free(low_leaf);
  return count;
}

// by default it's bit vector
uint32_t find_record_in_range(FILE *f, bv_t *bv, int low, int high)
{
  return _find_record_in_range_type(f, bv, NULL, false, low, high);
}

uint32_t find_record_in_range_id(FILE *f, list_t *ids, int low, int high)
{
  return _find_record_in_range_type(f, NULL, ids, true, low, high);
}
/*********************************************************************************
 * insert record
 *
 * public function for inserting records
 * note that the bpt_meta contains the original root
 *********************************************************************************/

void bpt_insert_record(FILE *f, int key, uint32_t row_index)
{
#ifdef UT_VERBOSE
  printf("BPT INSERT %d at %d", key, row_index);
#endif
  check_mem(f);
  fseek(f, 0, SEEK_SET);
  column_meta_t *cm = malloc(sizeof(column_meta_t));
  if (fread(cm, sizeof(column_meta_t), 1, f) != 1) {
    goto error;
  }

  if (cm->size == 0) {
    // make a new tree
    node_t *root = _make_new_leaf(f, cm);
    root->keys[0] = key;
    root->pointers[0] = row_index;
    root->pointers[BPT_SIZE - 1] = 0;
    root->size++;
    // want to write this to disk
    if (fwrite(root, sizeof(node_t), 1, f) != 1) {
      goto error;
    }
    // also need to update meta!
    // cm->size ++;
    // cm->next_free ++;
    // root is the current element
    cm->root = 0;
    if (fwrite(cm, sizeof(column_meta_t), 1, f) != 1) {
      goto error;
    }
    if (cm) free(cm);
    return;
  }

  // load root, it's always the first element
  node_t *root = malloc(sizeof(node_t));
  _load_node(f, root, cm->root);

  // find the leaf location to insert
  node_t *leaf = _find_leaf(f, root, key);

  // insert into leaf
  return _insert_node(f, cm, leaf, key, row_index);

error:
  perror("cannot access bptree file properly\n");
  exit(EXIT_FAILURE);
}


/* abstracted out for multiple use cases
 * finds the left most node that satisfies */
node_t *_find_leaf(FILE *f, node_t *root, int key)
{

#ifdef UT_VERBOSE
  printf("Finding leaf for key: %d\n", key);
#endif

  uint32_t counter = 0;
  uint32_t pos = 0;

  if (root == NULL) {
    if (V) { printf("Searching in empty tree!\n"); }
    if (TEST) { assert(false); }
    return NULL;
  }

  // just saved on the heap
  node_t cur_node = *root;
  while(!cur_node.is_leaf) // now go down the tree
  {
    // reset
    counter = 0;
    while(counter < cur_node.size) // go down the node
    {
      if (key > cur_node.keys[counter]) {
        counter ++;
      } else {
        break; // found a corresponding position
      }
    }
    // reset cur_node to move down
    pos = cur_node.pointers[counter];
    _load_node(f, &cur_node, pos);

  }
  node_t *result_node = malloc(sizeof(node_t)); // apparently you could just set the value like so
  memcpy(result_node, &cur_node, sizeof(node_t));

  // at leaf level
  return result_node;
}



/*********************************************************************************
 * insert node
 *
 * internal function for inserting interior nodes AND leaf nodes
 *********************************************************************************/


void _insert_node(FILE *f, column_meta_t *cm, node_t *n, int key, uint32_t pointer)
{
  /**********************
   * NO NEED TO SPLIT
   *********************/
  if (n->size < BPT_SIZE - 1)
  {
#ifdef UT_VERBOSE
    printf("BPT NO SPLIT");
#endif
    _add_entry_to_node(f, n, key, pointer);
    _write_cm(f, cm);
    return;
  }
  /**********************
   * SPLIT
   *********************/
  // If LEAF AND OVERFLOW (duplicates)!
  if (n->is_leaf == true && n->next != 0)
  {

#ifdef UT_VERBOSE
     printf("BPT SPLIT");
#endif
    node_t *next_node = malloc(sizeof(node_t));
    _load_node(f, next_node, n->next);

    if (next_node->parent == 0 && next_node->keys[0] == key)
    {
      // just append to the end
      // do not support more than one overflow node, way too much
      assert(next_node->size < BPT_SIZE - 1);

      next_node->keys[next_node->size] = key;
      next_node->pointers[next_node->size + 1] = pointer;
      next_node->size ++;
      // write to file
      _write_node(f, next_node);
      if(next_node) free(next_node);
      _write_cm(f, cm);
      // we are done
      return;
    }
  }
  // otherwise treat everything as the same
  // create a temporary array one size bigger so we don't need to
  // worry about the relative position of key and the split
  int temp_keys[BPT_SIZE];
  uint32_t temp_pointers[BPT_SIZE + 1];

  // figure out where to insert
  uint32_t pos = get_upper_bound_from_array(n->keys, n->size, key);

  // copy the memory over
  memcpy(temp_keys, n->keys, pos * sizeof(int));
  temp_keys[pos] = key;
  memcpy(&(temp_keys[pos+1]), &(n->keys[pos]), (n->size - pos) * sizeof(int));
  uint32_t offset;
  if (n->is_leaf) {
    offset = pos;
  } else {
    offset = pos + 1;
  }
  memcpy(temp_pointers, n->pointers, offset *sizeof(int));
  temp_pointers[offset] = pointer;
  memcpy(&(temp_pointers[offset + 1]), &(n->pointers[offset]), (n->size - pos) * sizeof(int));

  node_t *new_n = _make_new_leaf_or_node(f, cm, n->is_leaf);
  new_n->next = n->next;
  n->next = new_n->pos;
  n->size = BPT_SPLIT;
  memcpy(n->keys, temp_keys, sizeof(int) * (n->size));
  // for leaf nodes, the size of the split is different
  if (n->is_leaf) {

    new_n->size = BPT_SIZE - BPT_SPLIT;
    memcpy(new_n->keys, &(temp_keys[BPT_SPLIT]), sizeof(int) * (new_n->size));
    memcpy(n->pointers, temp_pointers, sizeof(int) * (n->size));
    memcpy(new_n->pointers, &(temp_pointers[n->size]), sizeof(int) * (new_n->size));


  }
  else {

    new_n->size = BPT_SIZE - 1 - BPT_SPLIT;
    memcpy(new_n->keys, &(temp_keys[BPT_SPLIT + 1]), sizeof(int) * (new_n->size));
    memcpy(n->pointers, temp_pointers, sizeof(int) * (n->size + 1));
    memcpy(new_n->pointers, &(temp_pointers[n->size + 1]), sizeof(int) * (new_n->size + 1));


    // also need to update the new node's children their new parent
    node_t *child = malloc(sizeof(node_t));
    for (uint32_t i = 0; i < new_n->size + 1; i++) {
      _load_node(f, child, new_n->pointers[i]);
      child->parent = new_n->pos;
      _write_node(f, child);
    }
    if(child) free(child);
  }

  pointer = new_n->pos;

  // if we are splititng root
  if(n->parent == 0) {
    // make new parent
    node_t *parent = _make_new_node(f,cm);
    parent->keys[0] = temp_keys[BPT_SPLIT];
    parent->pointers[0] = n->pos;
    parent->pointers[1] = new_n->pos;
    parent->size = 1;

    cm->root = parent->pos;
    n->parent = parent->pos;
    new_n->parent = n->parent;

    _write_node(f, new_n);
    _write_node(f, n);
    _write_node(f, parent);
    _write_cm(f, cm);

    // assert that we are not splitting at a duplicate
    // will support this in the future, but ignore for now (TODO)
    assert(temp_keys[BPT_SPLIT] != temp_keys[BPT_SPLIT + 1]);
    // done!
    return;

  }
  else {
    new_n->parent = n->parent;
    // copy the data over and erase the old values

    _write_node(f, new_n);
    _write_node(f, n);
    // load parent into the current node
    _load_node(f, n, n->parent);
    if (new_n) free(new_n);
    return _insert_node(f, cm, n, temp_keys[BPT_SPLIT], pointer);
  }

  assert(false);

}


/*******************************************************************************
 * bulk LOAD
 * assuming the array is already sorted! also that it's an empty file!
 *
 * first node is root
 *
 * the loading happens in stages, where each stage is a layer in the tree
 * - split each stage in arrays of size BPT_SIZE
 * - build the index arrays and link to subnodes
 * - link the parent over by aniticipation
 *
 * OPTIMIZATION TODOs:
 * - make the node not as full so less a chance for future splits!
 *******************************************************************************/

int bulk_load(FILE *f, int *sorted, uint32_t size)
{

  fseek(f, 0, SEEK_SET);
  column_meta_t *cm = malloc(sizeof(column_meta_t));
  if (fread(cm, sizeof(column_meta_t), 1, f) != 1) {
    perror("Read issues!\n");
    exit(EXIT_FAILURE);
  }

  // make leaf nodes, must be allocated for fwrite!
  int i = 0;
  int j = 0;
  int counter = 0;
  uint32_t total_nodes = 0;
  node_t *n = malloc(sizeof(node_t));
  n->is_leaf = true;
  n->size = BPT_SIZE - 1;

  // get the ceiling (TODO: ignoring duplciate corner cases)
  uint32_t num_nodes = get_ceil(size, BPT_SIZE - 1);
  debug("num_nodes size %d", num_nodes);

  cm->size = num_nodes;

  // keeps track of previous data
  int keys[num_nodes];
  int pointers[num_nodes];

  /***********
   * LOOP OVER LEAF LAYER
   ************/

  for (i = 0; i < num_nodes; i ++ ) {
    // storing the data so we don't need to read again
    keys[i] = sorted[counter];
    pointers[i] = i;

    // check if we are the last node
    if (counter > size - BPT_SIZE + 1) {
      n->size = size - counter;
      memcpy(n->keys, &(sorted[counter]), (size - counter) * sizeof(int));
      for (j = 0 ; counter < size; j++) {
        n->pointers[j] = counter;
        counter ++;
      }
      // NULL terminate
      n->next = 0;
    }
    else {
      // load in the keys
      memcpy(n->keys, &(sorted[counter]), (BPT_SIZE - 1) * sizeof(int));
      for (j = 0 ; j < BPT_SIZE - 1; j++) {
        n->pointers[j] = counter;
        counter ++;
      }
      n->size = BPT_SIZE - 1;
      n->next = total_nodes + 1;
      // make sure that there are no duplicates when we split the nodes!!!!
      if (sorted[counter] == sorted[counter -1]) {
        /* we need to put the previous value to the latter
         * TODO: make possible in the future
         * for now just kill it
         * a hack could be to change the BPT_SIZE and hope that it would go away*/
        if (TEST) {
          perror("sadly an duplicate is crossing your node!\n");
          exit(EXIT_FAILURE);
        }
      }
    }

    n->pos = total_nodes; // plus one for ROOT's allocation,
    n->parent = num_nodes + i/BPT_SIZE;

    // now we want to write to file!
    _write_node(f, n);
    total_nodes ++;
  }

  int new_itr_num_nodes = get_ceil(num_nodes, BPT_SIZE);

  // loop over for the interier nodes!
  n->is_leaf = false;
  n->next = 0;
  // otherwise
  while (new_itr_num_nodes != 1) {

    // populate the keys & pointers with the head of the leaf nodes
    // go back
    j = 0;
    counter = 0; // this counter is for the previous set of keys and pointers
    for (i = 0; i < new_itr_num_nodes; i++) {


      // check if we are at boundary case
      if (counter > num_nodes - BPT_SIZE) {
        // we want to skip one for the keys everytime
        /**
         * this is an annoying corner case
         *
         * here we need to duplicate a pointer, this should not damage the integrity of the tree however
         */
        if (num_nodes == 1) {
          memcpy(n->keys, &(keys[i * BPT_SIZE]), sizeof(int));
          n->pointers[0] = pointers[i * BPT_SIZE - 1];
          n->pointers[1] = pointers[i * BPT_SIZE];
        } else {
          memcpy(n->keys, &(keys[i * BPT_SIZE + 1]), (num_nodes - counter - 1) * sizeof(int));
          memcpy(n->pointers, &(pointers[i * BPT_SIZE]), (num_nodes - counter) * sizeof(int));
        }
        counter = num_nodes;
      }
      else {
        memcpy(n->keys, &(keys[i * BPT_SIZE + 1]), (BPT_SIZE - 1) * sizeof(int));
        memcpy(n->pointers, &(pointers[i * BPT_SIZE]), BPT_SIZE * sizeof(int));
        n->size = BPT_SIZE - 1;
        counter += BPT_SIZE;
      }

      n->pos = total_nodes;
      n->next = 0; // interior nodes doesn't need this
      n->parent = total_nodes + new_itr_num_nodes - i + i/BPT_SIZE;

      // write to disk
      _write_node(f, n);
      // now update the keys, we won't rewerite data that will be needed in the future
      keys[i] = keys[i * BPT_SIZE];
      pointers[i] = total_nodes;

      total_nodes ++;
    }
    num_nodes = new_itr_num_nodes;
    new_itr_num_nodes = get_ceil(num_nodes, BPT_SIZE);
  }
  // now we just one left, fill in the root node!
  assert(num_nodes <= BPT_SIZE);

  memcpy(n->keys, &(keys[1]), (num_nodes-1) * sizeof(int));
  memcpy(n->pointers, pointers, num_nodes * sizeof(int));
  n->parent = 0;
  n->pos = total_nodes;
  n->size = num_nodes - 1;
  _write_node(f, n);
  cm->next_free = total_nodes + 1;
  cm->root = n->pos;

  _write_cm(f, cm);
  free(cm);
  free(n);
  // done!
  return 0;

}

/*******************************************************************************
 * Do some math helper
 *
 *******************************************************************************/

size_t _get_bpt_file_cursor(uint32_t num)
{
  return sizeof(column_meta_t) + num * sizeof(node_t);
}


/*******************************************************************************
 * Allocation/Mem helper
 *
 *******************************************************************************/

node_t *_make_new_leaf(FILE *f, column_meta_t *cm)
{
  return _make_new_leaf_or_node(f, cm, true);
}

node_t *_make_new_node(FILE *f, column_meta_t *cm)
{
  return _make_new_leaf_or_node(f, cm, false);
}

// small helper functions
node_t *_make_new_leaf_or_node(FILE *f, column_meta_t *cm, bool is_leaf)
{

  node_t *new = malloc(sizeof(node_t));
  memset(new, 0, sizeof(node_t));
  new->is_leaf = is_leaf;
  new->parent = 0;
  new->next = 0;
  new->size = 0;
  new->pos = cm->next_free;

  // write to file
  // fseek(f, _get_bpt_file_cursor(cm->next_free), SEEK_SET);
  // fwrite(new, sizeof(node_t), 1, f);
  // don't write to file because there will be more edits!
  cm->next_free++;

  if (is_leaf == true) {
    cm->size++;
  }

  fseek(f, 0, SEEK_SET);
  fwrite(cm, sizeof(column_meta_t), 1, f);

  return new;
}


/*******************************************************************************
 * just some helpful loader functions!
 *
 *******************************************************************************/


/*
 * inserting a new entry to the node
 * note that the node must be sorted!
 * this is only called when the node does not need a split
 */

void _add_entry_to_node(FILE *f, node_t *node, int key, uint32_t pointer)
{
  uint32_t pos = get_upper_bound_from_array(node->keys, node->size, key);

  memmove(&(node->keys[pos+1]), &(node->keys[pos]), (node->size - pos) * sizeof(int));
  node->keys[pos] = key;

  if (node->is_leaf) {
    memmove(&(node->pointers[pos+1]), &(node->pointers[pos]), (node->size - pos) * sizeof(int));
    node->pointers[pos] = pointer;
  } else {
    // OMG THERE WAS A GLORIOUS BUG HERE THAT TOOK ME 2 HOURS TO FIGURE OUT
    // OVER WROTE THE STRUCT
    if (pos < node->size) {
      memmove(&(node->pointers[pos+2]), &(node->pointers[pos+1]), (node->size + 1 - pos) * sizeof(int));
    }
    node->pointers[pos+1] = pointer;
  }

  node->size ++;

  // also want to write the changes
  _write_node(f, node);
  return;
}

/*******************************************************************************
 * Some heavy lifting IO
 *
 *******************************************************************************/

void _write_cm(FILE *f, column_meta_t *cm)
{
  fseek(f, 0, SEEK_SET);
  fwrite(cm, sizeof(column_meta_t), 1, f);
  return;
}

void _load_node(FILE *f, node_t *node, uint32_t pos)
{
  if (VV) printf("Reading BPTREE node at pos %d\n", pos);
  return _node_io(f, node, pos, true);
}

void _write_node(FILE *f, node_t *node)
{
  return _node_io(f, node, node->pos, false);
}

void _node_io(FILE *f, node_t *node, uint32_t pos,  bool is_read)
{

  if (fseek(f, _get_bpt_file_cursor(pos), SEEK_SET) != 0) {
    goto error;
  }
  if (is_read == true) {
    if(fread (node, 1, sizeof(node_t), f) != sizeof(node_t)) {
      goto error;
    }
  } else {
    if(fwrite (node, 1, sizeof(node_t), f) != sizeof(node_t)) {
       goto error;
    }
  }

  return;

error:
  printf("ERR: trying to read pos %d\n", pos);
  perror("BTree NODE FILE!");
  exit(EXIT_FAILURE);
}

// for debugging!
void print_bpt(FILE *f)
{
  column_meta_t *cm = malloc(sizeof(column_meta_t));
  fseek(f, 0, SEEK_SET);
  fread(cm, sizeof(column_meta_t), 1, f);

  log_call("PRINT BPT");
  printf("The tree is size %d, with next free %d, with root %d.\n", cm->size, cm->next_free, cm->root);

  node_t *holder = malloc(sizeof(node_t));
  for (uint32_t i = 0; i < cm->next_free; i++) {
    _load_node(f, holder, i);
    print_bpt_node(holder);
  }
}

void print_bpt_node(node_t *holder)
{
  uint32_t j;
  if (holder->is_leaf) {
    printf("LEAF \t %d, parent %d\n", holder->pos, holder->parent);
  } else {
    printf("NODE \t %d, parent %d\n", holder->pos, holder->parent);
  }
  for (j=0; j< holder->size; j++) {
    printf("%d \t %d\n", holder->keys[j], holder->pointers[j]);
  }
  if (!holder->is_leaf) {
    printf("\t %d\n\n", holder->pointers[j]);
  } else {
    printf("\n");
  }
}
