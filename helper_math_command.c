#include "helper.h"
#include "dbg.h"

/*********************************************************************************
 * Math operation on variables
 *
 *********************************************************************************/
int math_op_on_var(variable_t *r, int *m1, int *m2, int (*op)(int,int))
{
  debug("MATH OP working");

  r->is_materialized = true;
  r->val = (int *)calloc(r->val_size, sizeof(int));
  check_mem(r->val);

  debug("Size of the varibal is %d", r->val_size);

  // iterate thru the values
  for (uint32_t i = 0; i < r->val_size; i ++) {
    r->val[i] = op(m1[i],m2[i]);
  }

  return 0;

error:
  log_err("MATH OP cannot allocate MEM\n");
  return 1;
}




/*********************************************************************************
 * Math function pointers for code abstraction!
 *********************************************************************************/

int fptr_add(int a, int b)
{
  return a+b;
}

int fptr_sub(int a, int b)
{
  return a-b;
}

int fptr_div(int a, int b)
{
  if (b == 0) {
    //TODO!!!FIXME this current hack doesn't work
    log_warn("DIVISION BY ZERO");
    return 0;
  }
  return a/b;
}

int fptr_mul(int a, int b)
{
  return a*b;
}


