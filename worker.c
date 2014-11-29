#include "server.h"

int db_worker(thread_arg *args)
{
  int s_con;
  int i, low, high;
  uint32_t u;
  int call_result;
  char *msg = NULL;
  bool has_var = false;
  char *end_ptr;

  bool has_error = false;
  bool is_loading = false;
  uint32_t col_size = 0;
  uint32_t row_size = 0;
  int cur_row = 0;
  char *load_col_names = NULL;
  int **ints_buffer = NULL;

  bool is_tuple = false;
  bool is_insert = false;
  bool is_delete = false;

  bool is_waiting_for_meta = true;
  uint32_t size_expected = sizeof(socket_meta_t); // used by both loading and tuple
  int v_count = 0;
  thread_pool_t *pool = args->pool;
  regex_t *r = args->r;

  while (1) {

    log_call("WORKER WAITING FOR NEW CONNECTION\n");

    //wait on the CV
    pthread_mutex_lock(&pool->mutex);
    while (pool->q_waiting == 0) {
      pthread_cond_wait(&pool->cv_socket_waiting, &pool->mutex);
    }

    // get the socket
    s_con = get_socket(pool);
    log_call("Connected to socket %d\n", s_con);

    pthread_mutex_unlock(&pool->mutex);

    char *data_buffer = malloc(MAX_COMMAND_BUFFER_SIZE * sizeof(char));
    // per session data
    list_t *vars = create_list(sizeof(variable_t));

    // allocate space for receiving arguments
    char ** input_args = (char **) malloc(MAX_ARG_NUM * sizeof(char *));
    for(i = 0; i < MAX_ARG_NUM; i++) {
      input_args[i] = (char *)malloc(MAX_INPUT_BYTES*sizeof(char));
    }

    do {
      //{{{ do worker set up
      call_result = safe_recv(s_con, data_buffer, size_expected);
      if (call_result != 0) {
        goto done;
      }

      // get socket_meta_t
      if (is_waiting_for_meta == true) {
        size_expected = ((socket_meta_t *)data_buffer)->expected_bytes;
        verbose("META received, new size expected is %u", size_expected);
        is_waiting_for_meta = false;

        // go to the next iteration!
        continue;
      }

      // must NULL terminate for other string processing
      data_buffer[size_expected] = '\0';
      is_waiting_for_meta = true;
      size_expected = sizeof(socket_meta_t);

      /**********************************************************************
       * LOADING
       ***********************************************************************/
      if (is_loading == true)
      {
        assert(col_size > 0);
        assert(row_size > 0);
        assert(load_col_names != NULL);
        assert(ints_buffer != NULL);

        /********************************
         * loading COLUME NAME data
         ********************************/
        if (cur_row == 0) {
          log_call("LOAD\n");
          verbose("-->COL NAMES\t");
          memcpy(load_col_names, data_buffer, col_size * sizeof(char) * MAX_NAME_SIZE);
          // TODO DELTE:
          for(i =0; i < col_size; i++) {
            printf("col name: %s\t", &(load_col_names[i * MAX_NAME_SIZE]));
          }
          cur_row ++;
          continue;
        }
        /********************************
         * loading INT data
         ********************************/
        else if (cur_row <= row_size) {
          verbose("-->INT VALS at row %d (total: %d)\n", cur_row, row_size);
          memcpy(ints_buffer[cur_row - 1], data_buffer, col_size * sizeof(int));

#ifdef VERBOSE
          for (int k = 0; k < col_size; k ++) {
            printf("%d\t", ints_buffer[cur_row - 1][k]);
          }
#endif
          cur_row ++;
          if (cur_row <= row_size) {
            continue;
          }
        }


        /********************************
         * NOW DO THE WRITING
         ********************************/
        call_result = load_helper(load_col_names, ints_buffer, row_size, col_size);
        if(call_result) goto done;

        // done!
        is_loading = false;
        cur_row = 0;

        // now free the allocated memeory
        free(load_col_names);
        for (u = 0; u < row_size; u ++) {
          free(ints_buffer[u]);
        }
        free(ints_buffer);
        continue;
      }
      /**********************************************************************
       * TUPLE work
       ***********************************************************************/
      else if (is_tuple == true)
      {
        debug("TUPLE, variable count is %d", v_count);
        list_t *result = create_list(sizeof(int));
        uint32_t tuple_int_count = 0;
        int *tuple_data = tuple_helper(vars, data_buffer, v_count, result, &tuple_int_count);
        dbg_assert(tuple_int_count != 0);
        call_result = safe_send(s_con, (char *)tuple_data, tuple_int_count * sizeof(int));
        if (call_result != 0) {
          goto done;
        }
        destroy_list(result);

        is_tuple = false;
        // clear previous
        memset(data_buffer, '\0', MAX_COMMAND_BUFFER_SIZE);
        // free memeory allocated
        continue;
      }
      /**********************************************************************
       * INSERT work
       * Note that the numbers are treated as names here, just overloading
       *   due to convenience
       ***********************************************************************/
      else if (is_insert == true)
      {
        debug("INSERT, column count is %d", v_count);
        // this must be multiples of two since it's one value per column
        dbg_assert(v_count % 2 == 0);
        char *col_name;
        int num_insert;
        // now loop over the values and call insert helper
        for (u = 0; u < v_count; u+=2) {
          col_name = &(data_buffer[u * MAX_NAME_SIZE]);
          num_insert = strtol(&(data_buffer[(u + 1) * MAX_NAME_SIZE]), NULL, 10);
          insert_helper(col_name, num_insert);
        }
        is_insert = false;
        continue;
      }

      /**********************************************************************
       * DELETE work
       *
       * right now only supporting unsorted, so could just simply switch the
       * last position over
       ***********************************************************************/
      else if (is_delete == true)
      {
        debug("DELETE, column count is %d", v_count);
        // note that data_bufer points to the vairable
        int *pos_val = NULL;
        int num_pos = get_pos_from_var(vars, data_buffer, &pos_val);

        // then remove from the columns
        // call each column independently
        for (u = 1; u < v_count; u ++) {
          delete_helper(&(data_buffer[u * MAX_NAME_SIZE]), pos_val, num_pos);
        }
        is_delete = false;
        continue;
      }
      /**********************************************************************
       * OTHER processing
       ***********************************************************************/
      else
      {

        if(!strncmp(data_buffer, "exit", 4)) {
          goto done;
        }

        if(!strncmp(data_buffer, "\n", 1)) {
          goto done;
        }


        if (match_regex(r, data_buffer, input_args))
        {
          // no match found
          msg = "please enter the correct syntax!\n";
          if (send(s_con, msg, strlen(msg), 0) < 0) {
            perror("cannot send");
            goto done;
          }
        }
        else // we have matching
        {
          // used by all components (not shared though)
          if (is_empty_str(input_args[IN_VAR]) == false) {
            has_var = true;
          } else {
            has_var = false;
          }
        }

        char *command = input_args[IN_COMMAND];
        log_call(">> %s\n", command);

        /*******************************************************************
         * CREATE
         *******************************************************************/
        if (strncmp(command, "create", 6)==0)
        {

          storage_t storage_type;
          if (strncmp(input_args[IN_ARG_TWO], "sorted", 4) == 0) {
            storage_type= SORTED;
          } else if (strncmp(input_args[IN_ARG_TWO], "unsorted", 8) == 0) {
            storage_type = UNSORTED;
          } else {
            storage_type = BPTREE;
          }

          make_new_column_file(input_args[IN_ARG_ONE], storage_type);

          continue;
        }

        /*******************************************************************
         * SELECT
         *******************************************************************/
        else if (strncmp(command, "select", 6) == 0)
        {
          // select must be acompanies by a variable
          assert (has_var == true);
          char *col_name = input_args[IN_ARG_ONE];
          column_t c; // create the column to be worked on
          init_col(&c, col_name);
          variable_t *new_var = initialize_variable(vars, input_args[IN_VAR], c.m.size);
          check_mem(new_var->bv);

          /**********************
           * get the whole column
           **********************/
          if (is_empty_str(input_args[IN_ARG_TWO]) == true) {
            mark_all_bv(new_var->bv);
            continue;
          }

          /**********************
           * select a set of values equal to a value
           **********************/
          else {
            low = strtol(input_args[IN_ARG_TWO],NULL,10);
            if (is_empty_str(input_args[IN_ARG_THREE]) != true) {
              high = strtol(input_args[IN_ARG_THREE],NULL,10);
            } else {
              high = low;
            }
            uint32_t result;

            /********* UNSORTED **********/
            if (c.m.type == UNSORTED) {
              result = mark_matching_bv_for_unsorted(&c, new_var->bv, low, high);
            }
            /********* SORTED **********/
            else if (c.m.type == SORTED) {
              result = mark_matching_bv_for_sorted(&c, new_var->bv, low, high);
            }
            /********* BPTREE **********/
            else {
              result = find_record_in_range(c.bpt_fp, new_var->bv, low, high);
              fclose(c.bpt_fp);
            }

            printf("just finished select");
            if (result <= 0) {
              printf("SELECT ERR: nothing was found in the select");
              assert(false);
            }
          }
          fclose(c.fp);
          continue;
        }
        /*******************************************************************
         * FETCH
         *******************************************************************/
        else if (strncmp(command, "fetch", 5)==0)
        {
          char *col_name = input_args[IN_ARG_ONE];
          int pos = strtol(input_args[IN_ARG_TWO], &end_ptr, 10);
          column_t c; // create the column to be worked on
          init_col(&c, col_name);

          // make sure there are two args!
          assert (is_empty_str(input_args[IN_ARG_TWO]) == false);

          /************************
           * STORING VARIABLE
           *************************/
          if (has_var == true) {
            variable_t *new_var = initialize_variable(vars, input_args[IN_VAR], c.m.size);

            /**********************************
             * READ from VARIABLE's
             * - BIT VECTOR: just copy the bit vector
             * - ROW ID: materialize (don't make sense not to)
             * no need to materialize
             **************************************/
            if (end_ptr - input_args[IN_ARG_TWO] == 0) {
              assert(pos == 0);
              // we are dealing with a variable here, find the varibale
              char *var_name = input_args[IN_ARG_TWO];
              variable_t *old_var = find_variable(vars, var_name);

              if (old_var->is_id) {
                new_var->is_materialized = true;
                new_var->val = fetch_matched_from_rowid(&c, old_var);
                new_var->val_size = get_list_size(old_var->ids);
              }
              else {
                cpy_bv(new_var->bv, old_var->bv);
              }
              new_var->is_fetched = true;

              // DONT CLOSE THE descripters --- might be used later
              strcpy(new_var->fetched_col.name, col_name);
              // free the file descripter
              fclose(c.fp);
              c.fp = NULL;
              if (c.m.type == BPTREE) {
                fclose(c.bpt_fp);
              }

              continue; // done with the iteration
            }
          }
          /********************************
           * NO VARIABLE, JUST PRINT
           *  -> we need to materialize
           *********************************/
          else {

            if (end_ptr - input_args[IN_ARG_TWO] == 0) {

              char *var_name = input_args[IN_ARG_TWO];
              variable_t *found_var = find_variable(vars, var_name);
              dbg_assert(found_var);
              int *result;
              int result_size;
              if (found_var->is_id) {
                result = fetch_matched_from_rowid(&c, found_var);
                result_size = get_list_size(found_var->ids);
              }
              else {
                //dbg_assert(found_var->is_fetched);
                result = fetch_matched_from_bv(&c, found_var->bv);
                result_size = get_bv_count(found_var->bv);
              }
              call_result = safe_send(s_con, (char *)result, sizeof(int) * result_size);
              if (call_result != 0) {
                goto done;
              }
              if(result) free(result);

              fclose(c.fp);
              if(c.m.type == BPTREE) {
                fclose(c.bpt_fp);
              }
              debug("Materialized and Fetched variable.\n");
              continue;
            }
            /****
             * JUST FETCHING ONE VALUE
             ****/
            else {
              // get the file we want to read
              fseek(c.fp, sizeof(int) * pos, SEEK_CUR);
              int val;

              fread(&val, sizeof(int), 1, c.fp);
              sprintf(msg, "%d", val);

              call_result = safe_send(s_con, msg, strlen(msg));
              if (call_result != 0) {
                goto done;
              }
              fclose(c.fp);
              debug("Materialized and Fetched value.\n");
              continue;
            }
          }
        }
        /*******************************************************************
         * LOAD
         *
         * You can expect that the first column of a "table" can be of any
         *  type, while others will be "unsorted."
         *******************************************************************/
        else if (strncmp(command, "load", 4)==0)
        {

          is_loading = true;
          col_size = (uint32_t)strtol(input_args[IN_ARG_ONE], &end_ptr, 10);
          row_size = (uint32_t)strtol(input_args[IN_ARG_TWO], &end_ptr, 10);
          debug("LOAD PREP\tCOL: %d \tROW: %d\n", col_size, row_size);
          load_col_names = malloc(sizeof(char) * MAX_NAME_SIZE * col_size);
          ints_buffer = malloc(sizeof(void *) * row_size);
          for (u = 0; u < row_size; u ++) {
            ints_buffer[u] = malloc(sizeof(int) * col_size);
          }
          continue;
        }
        /*******************************************************************
         * TUPLE
         *******************************************************************/
        else if (strncmp(command, "tuple", 5) == 0)
        {
          v_count = strtol(input_args[IN_ARG_ONE], &end_ptr, 10);
          log_call("TUPLE, first, we are expecting %d columns, the original call is %s\n", v_count, data_buffer);
          dbg_assert(v_count != 0);
          is_tuple = true;
        }

        /*******************************************************************
         * INSERT
         *******************************************************************/
        else if (strncmp(command, "insert", 6) == 0)
        {
          v_count = strtol(input_args[IN_ARG_ONE], &end_ptr, 10);
          log_call("INSERT, first, we are expecting %d columns\n", v_count);
          dbg_assert(v_count != 0);
          is_insert = true;
          continue;
        }

        /*******************************************************************
         * UPDATE
         *
         * Doesn't need complex receiving since it's limited in size
         *******************************************************************/
        else if (strncmp(command, "update", 6) == 0)
        {

          int *pos_val = NULL;
          uint32_t num_pos = get_pos_from_var(vars, input_args[IN_ARG_ONE], &pos_val);
          int val = (uint32_t)strtol(input_args[IN_ARG_THREE], &end_ptr, 10);
          update_helper(input_args[IN_ARG_TWO], pos_val, num_pos, val);
          continue;
        }
        /*******************************************************************
         * DELETE
         *******************************************************************/
        else if (strncmp(command, "delete", 6) == 0)
        {
          v_count = strtol(input_args[IN_ARG_ONE], &end_ptr, 10);
          log_call("DELETE, first, we are expecting %d columns\n", v_count);
          dbg_assert(v_count != 0);
          is_delete = true;
          continue;
        }
        /*******************************************************************
         * MATH Ops
         * DESIGN CHOICE:
         * - when a variable is fetched once the result is cached
         *******************************************************************/
        else if (strncmp(command, "add", 3) == 0 || strncmp(command, "sub", 3) == 0 || strncmp(command, "div", 3) == 0 || strncmp(command, "mul", 3) == 0 )
        {
#ifdef DEBUG
          assert (has_var == true); // FIXME: temporray, will support later
#endif
          int *m = NULL;
          variable_t *v[2];

          char *v1_name = input_args[IN_ARG_ONE];
          char *v2_name = input_args[IN_ARG_TWO];
          v[0] = find_variable(vars, v1_name);
          v[1] = find_variable(vars, v2_name);

          uint32_t size;

          // preprocess both variables
          for (i = 0; i< 2; i++) {
            if (v[i]->is_fetched) {
              size = get_bv_count(v[i]->bv);
              // load the vars in
              load_col(&(v[i]->fetched_col));
              m = fetch_matched_from_bv(&(v[i]->fetched_col), v[i]->bv);
              v[i]->is_materialized = true;
              v[i]->val = m;
            }
            else {
#ifdef DEBUG
              assert(v[i]->is_materialized);
              assert(v[i]->val != NULL);
              assert(v[i]->val_size > 0);
#endif
            }
          }
          variable_t *new_var = initialize_variable_materialized(vars, input_args[IN_VAR]);
          debug("Created a new variable for result of math of size %d", size);
          new_var->val_size = size;
          switch(*command) {
            case 'a':
              call_result = math_op_on_var(new_var, v[0]->val, v[1]->val, &fptr_add);
              break;
            case 's':
              call_result = math_op_on_var(new_var, v[0]->val, v[1]->val, &fptr_sub);
              break;
            case 'd':
              call_result = math_op_on_var(new_var, v[0]->val, v[1]->val, &fptr_div);
              break;
            case 'm':
              call_result = math_op_on_var(new_var, v[0]->val, v[1]->val, &fptr_mul);
              break;
          }
          if (call_result) goto done;
          continue;
        }

        /*******************************************************************
         * AGGREGATION Result
         *******************************************************************/
        else if (strncmp(command, "count", 4) == 0)
        {
#ifdef DEBUG
          assert(has_var == true);
#endif
          uint32_t count = 0;
          variable_t *var = find_variable(vars, input_args[IN_ARG_ONE]);
          if (var->is_fetched) {
            count = get_bv_count(var->bv);
          }
          else if (var->is_materialized) {
            count = var->val_size;
          }
          // now put this in a variable to be materialized....
          variable_t *new_var = initialize_variable_materialized(vars, input_args[IN_VAR]);

          new_var->is_materialized = true;
          new_var->val_size = 1;
          new_var->val = malloc(sizeof(int));
          check_mem(new_var->val);
          new_var->val[0] = count;
          continue;
        }
        else if (strncmp(command, "min", 3) == 0 || strncmp(command, "max", 3) == 0  || strncmp(command, "sum", 3) == 0 || strncmp(command, "avg", 3) == 0)
        {
#ifdef DEBUG
          assert(has_var == true);
#endif
          variable_t *var = find_variable(vars, input_args[IN_ARG_ONE]);
          load_col(&(var->fetched_col));

          int aggr_result;
          switch(command[1]) {
            case 'i':
              call_result = aggregate_vals(var, &fptr_min, false, &aggr_result);
              break;
            case 'a':
              call_result = aggregate_vals(var, &fptr_max, false, &aggr_result);
              break;
            case 'v':
              call_result = aggregate_vals(var, &fptr_add, true, &aggr_result);
              break;
            case 'u':
              call_result = aggregate_vals(var, &fptr_add, false, &aggr_result);
              break;
            default:
              log_err("Shouldn't be here");
              break;
          }
          if (call_result) goto done;

          variable_t *new_var = initialize_variable_materialized(vars, input_args[IN_VAR]);

          new_var->is_materialized = true;
          new_var->val_size = 1;
          new_var->val = malloc(sizeof(int));
          check_mem(new_var->val);
          new_var->val[0] = aggr_result;
          debug("Aggregation finished");

          continue;
        }
        /*******************************************************************
         * JOINs
         *
         * The matching is a little hacky since it's dependent on
         * sort, loop, hash and tree being the same len
         *******************************************************************/
        else if (strncmp(&(command[4]), "join", 4) == 0)
        {
          // need to parse the input by ','
          char *var_raw = input_args[IN_VAR];
          uint32_t var_raw_len = strlen(var_raw);
          char jv1_name[MAX_NAME_SIZE]; // jv for joined variable
          char jv2_name[MAX_NAME_SIZE];

          for (i = 0; i< var_raw_len; i++) {
            if (var_raw[i] == ',') {
              memcpy(jv1_name, var_raw, i);
              jv1_name[i]='\0';
              var_raw += i+1;
              memcpy(jv2_name, var_raw, var_raw_len - i-1);
              jv2_name[i]='\0';
              debug("The two variables for joining is %s %s", jv1_name, jv2_name);
              break;
            }
          }

          // get all the variables now
          variable_t *v1 = find_variable(vars, input_args[IN_ARG_ONE]);
          load_col(&(v1->fetched_col));
          variable_t *v2 = find_variable(vars, input_args[IN_ARG_TWO]);
          load_col(&(v2->fetched_col));
          debug("Joining %s (fetched from %s) with %s (fetched from %s)", input_args[IN_ARG_ONE], v1->fetched_col.name,input_args[IN_ARG_TWO], v2->fetched_col.name);
          variable_t *jv1 = initialize_rowid_variable(vars, jv1_name);
          variable_t *jv2 = initialize_rowid_variable(vars, jv2_name);

          int *m[2]; // materialized
          m[0] = NULL;
          m[1] = NULL;
          uint32_t m_size[2];
          uint32_t m_counter = 0;

          column_t *c[2];
          uint32_t s[2]; // size
          variable_t *v[2];
          v[0]=v1;
          v[1]=v2;
          // now materialize!
          // it makes sense to materialize early since n+m + n'*m' < n*m for most cases
          // read in the data
          for (i=0;i<2;i++) {
            // only do this if it's not a tree!

            c[i] = &(v[i]->fetched_col);
            check_mem(c[i]);

            // the logic is that we materialize the second one regardless of whether it's a tree if the
            // first one was a tree
            if(command[0] != 't' || c[i]->m.type != BPTREE || c[0]->m.type == BPTREE) {

              fseek(c[i]->fp, sizeof(column_meta_t), SEEK_SET);
              s[i] = c[i]->m.size;
              int* buffer = (int*) calloc(s[i], sizeof(int));
              size_t fread_count = fread(buffer, sizeof(int), s[i], c[i]->fp);

              dbg_assert(fread_count == s[i]);

              // first materialize and store their positions
              // times 2 because we need to store the location as well!
              m_size[i] = get_bv_count(v[i]->bv);
              m[i] = (int*) calloc(m_size[i] * 2, sizeof(int));

              m_counter = 0;
              for (uint32_t j = 0; j < s[i]; j++) {
                if (is_marked(v[i]->bv, j)) {
                  m[i][m_counter * 2] = buffer[j];
                  m[i][m_counter * 2 + 1] = j;
                  m_counter ++;
                }
              }

              dbg_assert(m_counter == m_size[i]);

              // only sort when need to
              if (command[0] == 's' && c[i]->m.type == UNSORTED) {
                qsort (m[i], m_counter, 2 * sizeof(int), compare_int);
              }
              if(buffer) free(buffer);
            }
          }


          switch(command[0]) {
            case 'l':
              loopjoin(jv1, jv2, m, m_size);
              break;
            case 's':
              sortjoin(jv1, jv2, m, m_size);
              break;
            case 'h':
              hashjoin(jv1, jv2, m, m_size);
              break;
            case 't':
              treejoin(jv1, jv2, v1, v2, m, m_size);
              break;
            default:
              log_err("Should not be here, check parser");
              return 1;

          }
          // should close the files that we have opened
          close_col_files(&(v1->fetched_col));
          close_col_files(&(v2->fetched_col));
          if (m[0]) free(m[0]);
          if (m[1]) free(m[1]);
          continue;
        }
      } // end of rejexed block
    } while (!has_error);
done:
    close(s_con);
    log_call("DONE with the current connection\n");
    if(data_buffer) free(data_buffer);
    if(vars) free(vars);
    if(input_args) {
      for(i = 0; i < MAX_ARG_NUM; i++) {
        if (input_args[i]) free(input_args[i]);
      }
      free(input_args);
    }
  }

error:
  log_err("something bad happened here!");
  return 1;
}
