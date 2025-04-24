#ifndef __FILE_OPS_H__
#define __FILE_OPS_H__

// C include
#include <stdarg.h>

// UNIX include
#include <sys/types.h>

enum FILEOPS_TYPES {
    FILEOPS_TYPE_CREATE = 0,
    FILEOPS_TYPE_REGISTER,
    FILEOPS_TYPE_READ,
    FILEOPS_TYPE_APPEND,
    FILEOPS_TYPE_REPLACE,
    FILEOPS_TYPE_RELEASE,
    FILEOPS_TYPE_MAX,
};

enum FILE_OPS_SYNC {
    FILE_OPS_ASYNC,
    FILE_OPS_SYNC,
    FILE_OPS_SYNC_MAX,
};

#define FILEOPS_NUM_ARGS_MIN        (4)     // appname fileio cmd filename
#define FILEOPS_NUM_ARGS_CREATE     (5)     // appname fileio cmd filename + mode
#define FILEOPS_NUM_ARGS_REGISTER   (4)     // appname fileio cmd filename
#define FILEOPS_NUM_ARGS_READ       (5)     // appname fileio cmd filename + length
#define FILEOPS_NUM_ARGS_APPEND     (5)     // appname fileio cmd filename + new string
#define FILEOPS_NUM_ARGS_REPLACE    (6)     // appname fileio cmd filename + new string + old string

typedef struct __fileops_args {
    const char *filename;
    union {
        struct {
            int filemode;
        } create;
        struct {
            char *buffer;
            int length;
        } read;
        struct {
            char *string;
        } append;
        struct {
            char *old_string;
            char *new_string;
        } replace;
    };
} fileops_args_t;

typedef struct __fileops_cmd {
    const char *cmdname;
    const int num_args;
} fileops_cmd_t;

extern const fileops_cmd_t fileops_cmd_args_list[FILEOPS_TYPE_MAX];

unsigned int fileops_create(const char *filename, fileops_args_t *p_args);

unsigned int fileops_register(const char *filename);
unsigned int fileops_read(const char *filename, fileops_args_t *p_args);
unsigned int fileops_append(const char *filename, fileops_args_t *p_args);
unsigned int fileops_replace(const char *filename, fileops_args_t *p_args);

unsigned int fileops_release(const char *filename);

#endif // __FILE_OPS_H__
