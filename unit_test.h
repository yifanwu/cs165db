#include <assert.h>
#include <string.h>
#include "config.h"
#include "parser.h"
#include "bit_vector.h"
#include "data_structures.h"
#include "list.h"
#include "client.h"
#include "helper.h"

void ut_list(uint32_t size);
void ut_bit_vector();
void ut_parser(void);

void ut_small_bpt();
void ut_bpt();
void ut_bpt_large();
void ut_bpt_bulk_load();

void ut_sorted_bounds(int size);
void ut_sorted();
void ut_unsorted(int size);
