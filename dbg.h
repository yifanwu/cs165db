#ifndef __dbg_h__
#define __dbg_h__

#include <stdio.h>
#include <errno.h>
#include <string.h>

#ifdef DEBUG
#define debug(M, ...) fprintf(stderr, "[DEBUG] %s:%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define log_call(M, ...) fprintf(stderr, "[CALL] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define dbg_assert(A) assert(A)
#else
#define debug(M, ...)
#define log_call(M, ...)
#define dbg_assert(A)
#endif

#ifdef VERBOSE
#define verbose(M, ...) fprintf(stderr, "VERBOSE %s:%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define verbose(M, ...)
#endif


#define clean_errno() (errno == 0 ? "None" : strerror(errno))

#ifdef KILLONERR
#define log_err(M, ...) { fprintf(stderr, "[ERROR] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__); exit(EXIT_FAILURE);}
#else
#define log_err(M, ...) fprintf(stderr, "[ERROR] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)
#endif

#define log_warn(M, ...) fprintf(stderr, "[WARN] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)


#define check(A, M, ...) if(!(A)) { log_err(M, ##__VA_ARGS__); errno=0; goto error; }

#define sentinel(M, ...)  { log_err(M, ##__VA_ARGS__); errno=0; goto error; }

#define check_mem(A) check((A), "Out of memory.")

#define check_debug(A, M, ...) if(!(A)) { debug(M, ##__VA_ARGS__); errno=0; goto error; }

#endif
