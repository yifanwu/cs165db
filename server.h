#ifndef SERVER_H_
#define SERVER_H_

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>

/* my own headers */
#include "data_structures.h"
#include "config.h"
#include "parser.h"
#include "bit_vector.h"
#include "threadpool.h"
#include "list.h"
#include "helper.h"
#include "dbg.h"

int db_worker(thread_arg *args);

#endif //SERVER_H_
