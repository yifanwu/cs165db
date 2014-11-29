#ifndef HELPER_H_
#define HELPER_H_
/*
 *
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <stdint.h>
 */

#include "data_structures.h"
#include "bit_vector.h"
#include "bptree.h"
#include "list.h"

/*********************************************************************************
 * JOIN
 *********************************************************************************/
int loopjoin(variable_t *jv1, variable_t *jv2, int **m, uint32_t *m_size);

int sortjoin(variable_t *jv1, variable_t *jv2, int **m, uint32_t *m_count);

int treejoin(variable_t *jv1, variable_t *jv2, variable_t *v1, variable_t *v2, int **m, uint32_t *m_count);

int hashjoin(variable_t *jv1, variable_t *jv2, int **m, uint32_t *m_size);
/*********************************************************************************
 * FETCH & TUPLE
 *********************************************************************************/

int *fetch_matched_from_rowid(column_t *c, variable_t *v);
int *fetch_matched_from_bv(column_t *c, bv_t *bv);

int *tuple_helper(list_t *vars, char *data_buffer, int v_count, list_t *result, uint32_t *result_size);

/*********************************************************************************
 * Aggregation
 *********************************************************************************/
int aggregate_vals(variable_t *v, int (*op)(int,int), bool is_avg, int *result_holder);

int fptr_max(int a, int b);

int fptr_min(int a, int b);

/*********************************************************************************
 * MATH OPs
 *********************************************************************************/

int math_op_on_var(variable_t *r, int *v1, int *v2, int (*op)(int,int));

/*********************************************************************************
 * UPDATE
 *********************************************************************************/
// converts the rowid list to an array, or bit vector to an array
// returns the number found and fills in the pointer
// int get_pos_from_var(list_t *vars, char *var_name, int *pos);
int get_pos_from_var(list_t *vars, char *var_name, int **pos);

// handles all that needed for a column insert
int insert_helper(char *col_name, int num_insert);

// unsorted insert write
int append_value_to_file(column_t *db_column, int val);

// sorted insert write
void insert_value_to_file(column_t *col, int index, int val);

int load_helper(char *col_names, int **ints_buffer, uint32_t row_size, uint32_t col_size);

// delete helper
int delete_helper(char *col_name, int *delete_pos, uint32_t num_pos);

// update
int update_helper(char *col_name, int *update_pos, uint32_t num_pos, int val);

/*********************************************************************************
 * VARIABLE
 *********************************************************************************/

int get_val_from_offset(FILE *f, uint32_t offset);

//variable_t *initialize_variable(global_meta_t *g, char *name, uint32_t size);
//variable_t *initialize_variable_materialized(global_meta_t *g, char *name);
//variable_t *initialize_rowid_variable(global_meta_t *g, char *name);
//variable_t *find_variable(global_meta_t *g, char *name);
variable_t *find_variable(list_t *vars, char *name);
variable_t *initialize_variable(list_t *g, char *name, uint32_t size);
variable_t *initialize_variable_materialized(list_t *g, char *name);
variable_t *initialize_rowid_variable(list_t *g, char *name);

/*********************************************************************************
 * SOCKET
 *********************************************************************************/


// unsorted
uint32_t mark_matching_bv_for_unsorted(column_t *c, bv_t *bv, int low, int high);

// sorted
uint32_t mark_matching_bv_for_sorted(column_t *c, bv_t *bv, int low, int high);

// bptree
int mark_matching_bv_from_bpt(FILE *f, unsigned int *bv, int key);


/*********************************************************************************
 * SOCKET
 *********************************************************************************/
// receiving
int safe_recv(int socket, char *data, uint32_t total_bytes_to_receive);

// sending
int safe_send(int socket, char *data, uint32_t total_bytes);
int _safe_send_helper(int socket, char *data, uint32_t total_bytes_to_send);

/*********************************************************************************
 * FILE OPEN
 *********************************************************************************/

// unsorted, sorted, and bptree create
void make_new_column_file(char *name, storage_t type);

FILE *_get_file(char *name, const char *op);
FILE *get_file_for_make(char *name);
FILE *get_file_for_read(char *name);
FILE *get_file_for_update(char *name);
char *get_file_name(char *name);
char *get_bpt_nodes_file_name(char *name);

/*********************************************************************************
 * MATH
 *********************************************************************************/
uint32_t get_ceil(uint32_t x, uint32_t y);

// do the search
uint32_t get_lower_bound(FILE *f, uint32_t size, int num);
uint32_t get_upper_bound(FILE *f, uint32_t size, int num);
uint32_t get_upper_bound_from_array(int *a, uint32_t size, int num);

// math helpers
int compare_int (const void * a, const void * b);
int compare_array (const void * a, const void * b);
int compare_malloced_array (const void * a, const void * b);


/*********************************************************************************
 * MISC
 *********************************************************************************/
// for regex
bool is_empty_str(char *in_str);
// hack for gdb breaking
void gdb_break(void);
void print_data_file(FILE *f);
void print_data_file_from_ptr(FILE *f);

int init_col(column_t *c, char *name);
int load_col(column_t *c);
void close_col_files(column_t *c);
int bounded_increment(int x, int bound);
/*********************************************************************************
 * FUNCTION POINTERS
 *********************************************************************************/
int fptr_add(int a, int b);
int fptr_mul(int a, int b);
int fptr_div(int a, int b);
int fptr_sub(int a, int b);
int fptr_inc(int a, int b);
#endif  // HELPER_H_
