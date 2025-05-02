#ifndef __PROCESS_DEF_H__
#define __PROCESS_DEF_H__
#include <sys/types.h>

#define MAX_PROCESS_CNT         (10)
#define MAX_PROCESS_NAME_LEN    (150)

enum process_state {
    PROCESS_STATE_NONE = 0,
    PROCESS_STATE_RUNNING,
    PROCESS_STATE_STOPPED,
    PROCESS_STATE_SIGNALED,
    PROCESS_STATE_UNEXPECTED,
    PROCESS_STATE_COMPLETE,
};

typedef struct __proc_entry {
    pid_t pid;
    char proc_name[MAX_PROCESS_NAME_LEN];
    unsigned int proc_state;
} proc_entry_t;

typedef struct __proc_list {
    proc_entry_t entry[MAX_PROCESS_CNT];
    unsigned int assigned_entry_bitmap;
    unsigned int nr_entry;
} proc_list_t;

#endif // __PROCESS_DEF_H__