/*****************************************************************************
 * server for Yifan's mini DB project for assignment 2
 *
 * parses input and calls the relevant database calls!
 *
 * plan of attack:
 * Part I: 1) basic parsing 2) data storage 3) retrieval
 * Part II: 4)threads 5)btree
 *
 * TODO: i guess all the value in main doesn't need to be malloc right?
 *****************************************************************************/
#include "server.h"

int main(void)
{
  //{{{ socket setup
  int s_init, s_con, optval;
  socklen_t t;
  uint32_t len = 0;
  struct sockaddr_un local, remote;

  if ((s_init = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    exit(1);
  }

  setsockopt(s_init,SOL_SOCKET,SO_REUSEADDR,&optval,8);

  local.sun_family = AF_UNIX;
  strcpy(local.sun_path, SOCK_PATH);
  unlink(local.sun_path);
  len = strlen(local.sun_path) + sizeof(local.sun_family);

  if (bind(s_init, (struct sockaddr *)&local, len) == -1) {
    perror("bind");
    exit(1);
  }

  if (listen(s_init, 5) == -1) {
    perror("listen");
    exit(1);
  }
  //}}} set up ends

  // {{{ globals setup

  // process input with regex
  regex_t *r = malloc(sizeof(regex_t));
  compile_regex(r, PASER_STR);
  thread_pool_t *pool = create_threadpool();

  thread_arg *args = malloc(sizeof(thread_arg));

  args->r = r;
  args->pool = pool;

  dispatch_threads(pool, &db_worker, args);

  //}}} global setup ends

  while(true) {

    printf("DB BOOTED, waiting for a connection...\n");
    t = sizeof(remote);

    if ((s_con = accept(s_init, (struct sockaddr *)&remote, &t)) == -1)
    {
      perror("accept");
      exit(1);
    }
    else
    {
      printf("RECEIVED SOCKET %d\n", s_con);
      // obtain the lock
      pthread_mutex_lock(&pool->mutex);
      add_new_socket(pool, s_con);
      pthread_cond_signal(&pool->cv_socket_waiting);
      pthread_mutex_unlock(&pool->mutex);
    }

  }
  if (remove(SOCK_PATH)) {
    perror("Failed to delete the socket file");
  };
  regfree(r);
  close(s_init);
  // we can't really clean up
  return 0;
}

