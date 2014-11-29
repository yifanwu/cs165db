#include "helper.h"
#include "dbg.h"

/*********************************************************************************
 * JOINs
 *
 * all joins take two variables that are fetched, whose size is contained in the bit vector
 * However, because the varibles are not materialized, we still need to read everything from emeory
 * again, this could be optimized in the future but will be like this for now
 *
 *********************************************************************************/

// Loop Join
int loopjoin(variable_t *jv1, variable_t *jv2, int **m, uint32_t *m_size)
{
  debug("Sort join in action");
  uint32_t i = 0;
  uint32_t j = 0;

  jv1->ids = create_list(sizeof(uint32_t));
  jv2->ids = create_list(sizeof(uint32_t));

  for (i = 0; i < m_size[0]; i++) {
    for (j = 0; j < m_size[1]; j++) {
      if(m[0][i*2] == m[1][j*2]) {
        debug("ADDED postion %d & %d for val %d", m[0][i * 2 + 1], m[1][j * 2 + 1], m[0][i*2]);
        // add the id
        append_list(jv1->ids, &(m[0][i*2+1]));
        append_list(jv2->ids, &(m[1][j*2+1]));
      }
    }
  }


  return 0;
}

/*********************************************************************************
 * sorted join
 * read everything in memeory (could potentially impelemnt something better)
 *
 * Note that that a key here is to maintain original mapping
 * This is also more expensive so we want to materialize earlier. It's better to
 *   sort less data than to sort more
 *
 * A potential optimization is to memorize the result but it's a bit difficult to
 *   maintain so outside the scope of this assignment
 *
 * Storing the two val in an array to avoid repeated code
 *********************************************************************************/
int sortjoin(variable_t *jv1, variable_t *jv2, int **m, uint32_t *m_size)
{
  debug("Sort join in action");
  uint32_t i = 0;
  uint32_t j = 0;
  uint32_t itr = 0; // iterater for duplciates

  debug("about to do the loop");

  ////TODO delete later:
  //for(int y=0; y<2;y++) {
  //  for (int x=0;x<m_size[y]; x++)
  //  {
  //    printf("%d\t", m[y][x*2]);
  //  }
  //  printf("\n");
  //}
  // TOOD: test extensively for corner cases
  i = 0;
  j = 0;
  while(i < m_size[0] && j < m_size[1])
  {
    debug("Currently at %d, %d", i, j);

    if (m[0][i*2] < m[1][j*2]) {
      i++;
    }
    else if  (m[0][i*2] > m[1][j*2])  {
      j++;
    }
    else { // we found a match!
      append_list(jv1->ids, &(m[0][i*2+1]));
      append_list(jv2->ids, &(m[1][j*2+1]));

      //debug("ADDED postion %d & %d for val %d", m[0][i * 2 + 1], m[1][j * 2 + 1], m[0][i*2]);
      // but we need to also check previous values since there might be duplicates

      // fix 0 and move the other and this is sufficient
      itr = j-1;
      while(itr > 0 && m[0][i*2] == m[1][itr*2]) {
        append_list(jv1->ids, &(m[0][i*2+1]));
        append_list(jv2->ids, &(m[1][itr*2+1]));
        //debug("UP ADDED postion %d & %d for val %d", m[0][i * 2 + 1], m[1][itr * 2 + 1], m[0][i*2]);
        itr --;
      }
      // now need to check down
      itr = j+1;
      while(itr < m_size[1] && m[0][i*2] == m[1][itr*2]) {
        append_list(jv1->ids, &(m[0][i*2+1]));
        append_list(jv2->ids, &(m[1][itr*2+1]));
        //debug("DOWN ADDED postion %d & %d for val %d", m[0][i * 2 + 1], m[1][itr * 2 + 1], m[0][i*2]);
        itr ++;
      }

      j = itr-1;

      // ready to move on
      // but just move the left cursor, which is i
      i++;
    }
  }
  return 0;
}

/*********************************************************************
 * tree join
 *
 * assumes that there is a tree
 * need to find which one is the tree
 * all non-tree values are materialized
 *   if both are trees, then assume the second one is materialized
 ********************************************************************/
int treejoin(variable_t *jv1, variable_t *jv2, variable_t *v1, variable_t *v2, int **m, uint32_t *m_size)
{
  debug("TREE JOIN");
  uint32_t result_count = 0;
  int *vals = NULL;
  int val_size = 0;
  FILE *tree_fp = NULL;
  bv_t *tree_bv = NULL;
  variable_t *tree_v = NULL;
  variable_t *other_v = NULL;

  // check if v1 is tree, if it's not, then v2 must be
  if (v1->fetched_col.m.type == BPTREE) {
    // now just look over v2 and search for values in v1
    // v2 also must be materialized
    if (v2->is_materialized == false) log_err("v2 not materialized");
    val_size = m_size[1];
    vals = m[1];
    tree_fp = v1->fetched_col.bpt_fp;
    tree_bv = v1->bv;
    tree_v = jv1;
    other_v = jv2;
  }
  else {
    if (v2->fetched_col.m.type != BPTREE) log_err("Neither type are trees!");
    // now v2 is the tree and v1 is materialized!
    val_size = m_size[0];
    vals = m[0];

    tree_fp = v2->fetched_col.bpt_fp;
    tree_bv = v2->bv;
    tree_v = jv2;
    other_v = jv1;
  }

  if (val_size == 0) log_err("Joining value zero!");
  list_t *id_list = NULL;

  for (uint32_t i = 0; i < val_size; i++) {
    if (!(i > 0 && vals[(i-1)*2] == vals[i*2])) {
      if (id_list) destroy_list(id_list);
      id_list = create_list(sizeof(int));
      debug("Finding value %d in tree at position %d", vals[i*2], i);
      // TODO: memorize the result and increment i
      result_count = find_record_in_range_id(tree_fp, id_list, vals[i*2], vals[i*2]);
    }
    if (result_count != 0) {
      // now we want to append the values to the result
      for (uint32_t j = 0; j < result_count; j++) {
        // check if this result is in range
        uint32_t pos = *(int *)get_list_val(id_list, j);
        if (is_marked(tree_bv, pos)) {
          append_list(tree_v->ids, get_list_val(id_list, j));
          append_list(other_v->ids, &(vals[i*2+1]));
        }
      }
    }
  }

  return 0;
}

/*********************************************************************
 * hash join
 *
 * static 2 pass
 * - hashing on the smaller table
 * - hash value must contain the original value AND its position
 * - hash function: just the last few bits
 *
 * don't implement grace join yet because since the values are in memeory anyway
 *   it's kinda weird to pretend that the hash table cannot fit into memeory!
 ********************************************************************/
int hashjoin(variable_t *jv1, variable_t *jv2, int **m, uint32_t *m_size)
{
  debug("Hash JOIN in action");
  // pick the smaller one for the hash table
  uint32_t i = 0;
  uint32_t min_size = 0;
  uint32_t other_size = 0;
  int *hash_val = NULL;
  int *other_val = NULL;
  variable_t *hash_v = NULL;
  variable_t *other_v = NULL;

  if (m_size[0]>m_size[1]) {
    min_size = m_size[1];
    other_size = m_size[0];
    hash_val = m[1];
    other_val = m[0];
    // TODO: buggy
    hash_v = jv2;
    other_v = jv1;
  }
  else {
    min_size =m_size[0];
    other_size = m_size[1];
    hash_val = m[0];
    other_val = m[1];
    hash_v = jv1;
    other_v = jv2;
  }

  // now calculate the number of bits for the hash
  uint32_t temp = get_ceil(min_size, AVG_HASH_BUCKET_SIZE);

  // hash mask
  uint32_t hash_mask = 0;
  uint32_t num_bits = 0;
  while(temp > 0) {
    temp >>= 1;
    num_bits ++;
  }
  hash_mask = (1 << num_bits) - 1;
  debug("the hash_mask is %d", hash_mask);
  // now do the first pass
  uint32_t num_buckets = 1 << num_bits;
  // should set val to 0 as well
  uint32_t *remap = calloc(num_buckets, sizeof(int));
  for (i = 0; i < min_size; i++) {
    debug("origianl val is %d, hashed is %d", hash_val[i * 2], hash_val[i * 2] & hash_mask);
    remap[hash_val[i * 2] & hash_mask] ++;
  }

  debug("Expecting %d buckets and reading %d bits", num_buckets, num_bits);

  // now we need to calculate the offsets
  uint32_t sum = 0;
  uint32_t temp_update = 0;
  for (i = 0; i < num_buckets; i++) {
    temp_update = remap[i];
    remap[i] = sum;
    sum += temp_update;
  }
  dbg_assert(sum == min_size);

  uint32_t *remap_count = calloc(num_buckets, sizeof(int));
  // keeps track of where to add!
  memcpy(remap_count, remap, sizeof(int) * num_buckets);

  for (i = 0; i < num_buckets; i++) {
    debug("%d stores: %d and %d\t", i, remap[i], remap_count[i]);
  }


  // now we are ready to build the hash table
  // it will be in groups of 2 integers
  int *hash_table = malloc(sizeof(int) * min_size * 2);
  for (uint32_t i = 0; i < min_size; i++) {
    int index = remap_count[hash_val[i * 2] & hash_mask];
    hash_table[index*2] = hash_val[i * 2];
    hash_table[index*2 + 1] = hash_val[i * 2 + 1];
    // increment the location pointer!
    remap_count[hash_val[i * 2] & hash_mask] ++;
  }

  // now let's check the values, yay!
  for (i = 0; i < other_size; i++) {
    uint32_t hash_index = remap[other_val[i * 2] & hash_mask];
    uint32_t hash_index_max = remap_count[other_val[i * 2] & hash_mask];
    // now go down the hash_index for equal
    while (hash_index < hash_index_max) {
      // now go down and find it!
      if (hash_table[hash_index * 2] == other_val[i*2]) {
        // found, add position!
        //debug("Found the position!");
        append_list(hash_v->ids, &(hash_table[hash_index * 2 + 1]));
        append_list(other_v->ids, &(other_val[i*2 + 1]));
      }
      hash_index++;
    }
  }

  free(remap);
  free(remap_count);
  free(hash_table);
  return 0;
}
