#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "config.h"

typedef struct {
  pthread_t threads[THREAD_POOL_SIZE]; //pointer to threads
  
  // this is circular array for a queue
  int q_head;
  int q_waiting;
  int sockets[THREAD_POOL_MAX_QUEUE_SIZE];
  
  pthread_mutex_t mutex;
  pthread_cond_t cv_socket_waiting;

} thread_pool_t;

// main functions
thread_pool_t *create_threadpool(void);
int dispatch_threads(thread_pool_t *pool, void *work, void *args);

// helper functions
int add_new_socket(thread_pool_t *pool, int socket);
int get_socket(thread_pool_t *pool);

#endif //THREADPOOL_H_
