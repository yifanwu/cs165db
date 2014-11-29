#ifndef CONFIG_H_
#define CONFIG_H_

//#define V false // GDB fail
//#define VV false
//#define VC false
//#define VVC false
#define V true // GDB fail
#define VV false
#define VC false
#define VVC false
#define TEST false
#define IOV 0
#define SOCK_PATH "db_sock"
// 60 is a rather arbitrary choice, should be enough
#define MAX_ARG_NUM 7
#define MAX_INPUT_BYTES 32
// this how big one of the commands from the test was
#define MAX_COMMAND_BUFFER_SIZE 1 << 16
//!!!NOTE that this is in the unit of INTS, not bytes
#ifdef DEBUG
#define FILE_BLOCK_SIZE 128
#define AVG_HASH_BUCKET_SIZE 8
#else
#define FILE_BLOCK_SIZE 128
#define AVG_HASH_BUCKET_SIZE 16
#endif
// assuming integer is 4 bytes and char is one
// TODO: make int32_t
#define MSG_SIZE 250
#define DEFAULT_DYN_ARR_SIZE 3
#define COL_META_OFFSET 2 // in terms of integers
#define MAX_NAME_SIZE 16
#define THREAD_POOL_SIZE 5
#define THREAD_POOL_MAX_QUEUE_SIZE 100
#define INT_STR_WIDTH 16 // can't be bigger, might have overflow
// for b plus tree, 4 for testing purposes
#define BPT_SIZE 128 // doesn't pass if this number is too small
#define BPT_SPLIT BPT_SIZE/2

#define DB_META "db_meta.data"
#define DB_META_TEMP "db_meta_temp.data"
#define DATA_EXTENTION "data"
#define BPT_RAW "nodes"
// TODO: what is this again?
#define DB_CONTENT "db_raw"

// will be increased if not enough
#define READLINE_BUFFER_SIZE 256

// does not allow for white space
#define PASER_STR "([A-z0-9,_]*)=?(max|min|avg|sum|count|sub|add|div|mul|hashjoin|loopjoin|sortjoin|treejoin|select|fetch|create|load|insert|delete|update|tuple)\\(\"?([A-z0-9_]*)\"?,?\"?([A-z0-9+_]*)\"?,?([A-z0-9]*)\\)"

// defining a couple error codes
#define ERR_MEM 1
#define ERR_FILE_NAME 2
#define ERR_FILE_CONTENT 3

#define LOAD_META_MSG "file_size:"
#define LOAD_END_MSG "done_loading"

#endif //CONFIG_H_
