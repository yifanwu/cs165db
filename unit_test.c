#include "unit_test.h"
int main(void)
{
  printf("UTILITY UNIT TEST BEGINS... GOOD LUCK!\n");
  ut_parser();
  ut_bit_vector();
  ut_list(600);

  printf("DATA UNIT TEST BEGINS!\n");
  ut_small_bpt();
  ut_bpt();
  ut_bpt_large();
  ut_bpt_bulk_load();
  ut_sorted();
  ut_sorted_bounds(10);
  ut_unsorted(6);

  printf("\nCongratulations! You have passed all the unit tests!\n\n");
  printf("\nCongratulations! You have passed all the unit tests!\n\n");
}
