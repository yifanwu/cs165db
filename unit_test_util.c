#include "unit_test.h"

void ut_parser(void)
{
  printf("Unit Testing Parser!\n");

  // setting up
  regex_t r;
  const char * find_text;

  char ** input_args = (char **) malloc(MAX_ARG_NUM * sizeof(char *));
  for(int i=0; i<MAX_ARG_NUM; i++) {
      input_args[i] = (char *)malloc(MAX_INPUT_BYTES*sizeof(char));
  }
  compile_regex(& r, PASER_STR);

  // execution
  find_text = "select(C,10,20)";
  printf ("PARSER: '%s'\n", PASER_STR);

  printf ("Parsing '%s'\n", find_text);
  match_regex(& r, find_text,input_args);

  assert(strncmp(input_args[1],"select",5)==0);
  assert(strncmp(input_args[2],"C",1)==0);
  assert(strncmp(input_args[3],"10",2)==0);
  assert(strncmp(input_args[4],"20",2)==0);

  // execution
  find_text = "inter=select(C,x)";
  printf ("Parsing '%s'\n", find_text);
  match_regex(& r, find_text,input_args);

  assert(strncmp(input_args[0],"inter",5)==0);
  assert(strncmp(input_args[1],"select",5)==0);
  assert(strncmp(input_args[2],"C",1)==0);
  assert(strncmp(input_args[3],"x",1)==0);


  // execution
  find_text = "select(hi,var)";
  printf ("Parsing '%s'\n", find_text);
  match_regex(& r, find_text,input_args);
  assert(strncmp(input_args[1],"select",5)==0);
  assert(strncmp(input_args[2],"hi",2)==0);
  assert(strncmp(input_args[3],"var",3)==0);
  printf("success!\n\n");

  // execution
  find_text = "load(6)";
  printf ("Parsing '%s'\n", find_text);
  match_regex(& r, find_text,input_args);
  assert(strncmp(input_args[1],"load",4)==0);
  assert(strncmp(input_args[2],"6",1)==0);


  // execution
  find_text = "create(friends,\"unsorted\")";
  printf ("Parsing '%s'\n", find_text);
  match_regex(& r, find_text,input_args);
  assert(strncmp(input_args[1],"create", 5)==0);
  assert(strncmp(input_args[2],"friends", 5)==0);
  assert(strncmp(input_args[3],"unsorted", 8)==0);
  printf("success!\n\n");

  // execution
  find_text = "create(friends,\"sorted\")";
  printf ("Parsing '%s'\n", find_text);
  match_regex(& r, find_text,input_args);
  assert(strncmp(input_args[1],"create", 5)==0);
  assert(strncmp(input_args[2],"friends", 5)==0);
  assert(strncmp(input_args[3],"sorted", 6)==0);
  printf("success!\n\n");

  // execution
  find_text = "create(friends,\"unsorted\")";
  printf ("Parsing '%s'\n", find_text);
  match_regex(& r, find_text,input_args);
  assert(strncmp(input_args[1],"create",5)==0);
  assert(strncmp(input_args[2],"friends",5)==0);
  assert(strncmp(input_args[3],"unsorted",8)==0);
  printf("success!\n\n");

  // execution
  find_text = "insert(friends,10)";
  printf ("Parsing '%s'\n", find_text);
  match_regex(& r, find_text,input_args);
  assert(strncmp(input_args[1],"insert",5)==0);
  assert(strncmp(input_args[2],"friends",5)==0);
  assert(strncmp(input_args[3],"10",2)==0);
  printf("success!\n\n");

  // execution
  find_text = "select(C,20,30)";
  printf ("Parsing '%s'\n", find_text);
  match_regex(& r, find_text,input_args);
  assert(strncmp(input_args[1],"select",5)==0);
  assert(strncmp(input_args[2],"C",1)==0);
  assert(strncmp(input_args[3],"20",2)==0);
  assert(strncmp(input_args[4],"30",2)==0);
  printf("success!\n\n");

  // execution
  find_text = "create(t2a,\"b+tree\")";
  printf ("Parsing '%s'\n", find_text);
  match_regex(& r, find_text,input_args);
  assert(strncmp(input_args[1],"create",5)==0);
  assert(strncmp(input_args[2],"t2a",1)==0);
  assert(strncmp(input_args[3],"b+tree",6)==0);
  printf("success!\n\n");


  // execution
  find_text = "add(t2,t3)";
  printf ("Parsing '%s'\n", find_text);
  match_regex(& r, find_text,input_args);
  assert(strncmp(input_args[1],"add",3)==0);
  assert(strncmp(input_args[2],"t2",2)==0);
  assert(strncmp(input_args[3],"t3",2)==0);
  printf("success!\n\n");


  // execution
  find_text = "r_results,s_results=hashjoin(join_input1,join_input2)";
  printf ("Parsing '%s'\n", find_text);
  match_regex(& r, find_text,input_args);
  assert(strncmp(input_args[0],"r_results,s_results", 19)==0);
  assert(strncmp(input_args[1],"hashjoin", 8)==0);
  assert(strncmp(input_args[2],"join_input1",11)==0);
  assert(strncmp(input_args[3],"join_input2",11)==0);
  printf("success!\n\n");


  regfree (& r);

  // prevent memory leak!
  for(int i=0;i<4;i++) {
    free(input_args[i]);
  }
  free(input_args);

  printf("SUCCESS!\n");

  return;

}

void ut_bit_vector() {
  printf("Bit vector unit testing!\n");
  bv_t *bv = create_bv(5);
  mark_bv(bv, 1);
  assert(is_marked(bv, 1)==true);
  unmark_bv(bv, 1);
  assert(is_marked(bv, 1)==false);
  mark_bv(bv, 2);
  mark_bv(bv, 4);
  assert(is_marked(bv, 2)==true);
  assert(is_marked(bv, 4)==true);

  bv =create_bv(500);
  assert(is_marked(bv, 398)==false);
  mark_bv(bv, 398);
  assert(is_marked(bv, 398)==true);

  printf("Checking mark_bv_by_range\n");
  mark_bv_by_range(bv, 100, 200);

  for (uint32_t i = 100; i < 201; i ++) {
    // printf("At index %d\n", i);
    assert(is_marked(bv, i)==true);
  }

  destroy_bv(bv);

  printf("BIT VECTOR SUCCESS!\n");

  return;
}

void ut_list(uint32_t size)
{
  printf("--- LIST ---\n");
  list_t *list = create_list(sizeof(int));

  for (int i = 0; i < size * 3; i++) {
    append_list(list, &i);
  }

  for (uint32_t i = 0; i < size * 3; i++) {
    if (((int*)list->data)[i] != i) {
      printf("Comparing %d with %d\n", ((int*)list->data)[i], i);
      assert(false);
    }
  }

  destroy_list(list);

  return;
}


