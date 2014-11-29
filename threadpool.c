#include "threadpool.h"

thread_pool_t *create_threadpool(void) 
{
  thread_pool_t *pool = malloc(sizeof(thread_pool_t));
  // set to default values
  pthread_mutex_init(&pool->mutex, NULL);
  pthread_cond_init(&pool->cv_socket_waiting, NULL);
  pool->q_head = 0;
  pool->q_waiting = 0;


  return pool;
}

int dispatch_threads(thread_pool_t *pool, void *work, void *args)
{

  for (int i = 0; i < THREAD_POOL_SIZE; i ++) {
    if (pthread_create(&(pool->threads[i]), NULL, work, args) == 1) {
      perror("Error creating the thread!\n");
      return 1;
    }
  }

  return 0;

}


/*******************************************************
 * queue helpers
 ******************************************************/
 
int add_new_socket(thread_pool_t *pool, int socket) 
{
  int pos = pool->q_head + pool->q_waiting;

  pos %= THREAD_POOL_MAX_QUEUE_SIZE;

  pool->sockets[pos] = socket;

  pool->q_waiting ++;

  return 0;
}

// returns the socket
int get_socket(thread_pool_t *pool) 
{
  int socket;
  // get it
  socket = pool->sockets[pool->q_head];

  // pop it
  pool->q_head ++;
  pool->q_head %= THREAD_POOL_MAX_QUEUE_SIZE;
  

  pool->q_waiting --;

  return socket;

}


/*******************************************************
 * memory processing helpers
 ******************************************************/

void destroy_threadpool(thread_pool_t *pool) 
{
  pthread_mutex_destroy(&pool->mutex);
  pthread_cond_destroy(&pool->cv_socket_waiting);
  free(pool);

  return;
}

