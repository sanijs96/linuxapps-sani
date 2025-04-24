#ifndef __FILE_OPS_DEF__
#define __FILE_OPS_DEF__

#define MAX_FILELIST_ENTRY      (10)
#define MAX_FILENAME_LENGTH         (100)

typedef struct __filelist_entry {
    int fd;
    char filename[MAX_FILENAME_LENGTH];
} filelist_entry_t;

typedef struct __filelist {
    filelist_entry_t entry[MAX_FILELIST_ENTRY];
    unsigned int nr_entry;
} filelist_t;

#define __handle_file_ops(ops_fn, err_fn) ({    \
    unsigned int result;                        \
    result = SUCCESS;                           \
    if ((ops_fn == -1) && (err_fn == -1)) {     \
        result = FAILURE;                       \
    }                                           \
    result;                                     \
})

#endif // __FILE_OPS_DEF__
