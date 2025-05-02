#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#include "process.h"
#include "../common/include/common.h"
#include "../common/include/bitops.h"

static proc_list_t proc_list = {
    .entry = {0, },
    .nr_entry = 0,
};

static unsigned int __allocate_process_list_entry(proc_list_t *p_list)
{
    unsigned int bitentry_idx;

    bitentry_idx = find_first_bit_zero(p_list->assigned_entry_bitmap);
    if (bitentry_idx < MAX_PROCESS_CNT) {
        p_list->nr_entry++;
        p_list->assigned_entry_bitmap |= (1 << bitentry_idx);
    }

    return bitentry_idx;
}

static unsigned int __register_process_entry(proc_entry_t *p_entry, char *procname,
                                                char **args, int num_args)
{
    p_entry->pid = getpid();
    strncpy(p_entry->proc_name, procname, MAX_PROCESS_NAME_LEN);
    p_entry->proc_state = PROCESS_STATE_RUNNING;

    printf("executing process: < %s", procname);
    for (int idx = 0; idx < num_args; idx++) {
        printf(", %s", args[idx]);
    }
    printf(" >\n");

    if (execv(procname, args) == -1) {
        printf("process execution failed, errno :%u\n", errno);
        exit (-1);
    }

    // returns only on error
    return FAILURE;
}

static void __free_process_entry(unsigned int entry_idx)
{
    proc_entry_t *p_entry;
    p_entry = &(proc_list.entry[entry_idx]);

    p_entry->proc_state = PROCESS_STATE_NONE;
    proc_list.assigned_entry_bitmap &= ~(1 << entry_idx);
    proc_list.nr_entry--;
}

static void __dump_process_list(proc_list_t *p_list)
{
    printf("[proc list] dump: bmap 0x%x, nr_entry %u\n",
            p_list->assigned_entry_bitmap, p_list->nr_entry);

    for (int i = 0; i < MAX_PROCESS_CNT; i++) {
        printf("\t[%u] " "name %s, " "pid %u, " "state %u\n",
                i, p_list->entry[i].proc_name,
                p_list->entry[i].pid,
                p_list->entry[i].proc_state);
    }
}

unsigned int process_add(char *proc, char **args, int num_args)
{
    unsigned int res;
    unsigned int new_proc_entry_idx;
    pid_t pid_chld;

    res = SUCCESS;

    // check slot
    if (proc_list.nr_entry == MAX_PROCESS_CNT) {
        printf("process list is full\n");
        res = FAILURE;
        goto exit;
    }

    // allocate entry
    new_proc_entry_idx = __allocate_process_list_entry(&proc_list);

    if (new_proc_entry_idx == MAX_PROCESS_CNT) {
        printf("process list error\n");
        __dump_process_list(&proc_list);
        res = FAILURE;
        goto exit;
    }

    pid_chld = fork();

    if (pid_chld == 0) {
        res = SUCCESS;
    }
    else if (pid_chld > 0) {
        proc_entry_t *p_entry;
        p_entry = &(proc_list.entry[new_proc_entry_idx]);
        // register entry context
        res = __register_process_entry(p_entry, proc, args, num_args);
    }
    else {
        printf("process create failed, errno :%u\n", errno);
        res = FAILURE;
    }

exit:
    return res;
}

static void __update_process_result(proc_entry_t *p_entry)
{
    if ((p_entry->proc_state == PROCESS_STATE_NONE) ||
        (p_entry->proc_state == PROCESS_STATE_COMPLETE)) {
        return;
    }

    int status;
    if (waitpid(p_entry->pid, &status, WNOHANG) == p_entry->pid) {
        if (WIFEXITED(status)) {
            p_entry->proc_state = PROCESS_STATE_COMPLETE;
        }
        else if (WIFSTOPPED(status)) {
            p_entry->proc_state = PROCESS_STATE_STOPPED;
        }
        else if (WIFSIGNALED(status)) {
            p_entry->proc_state = PROCESS_STATE_SIGNALED;
        }
        else {
            p_entry->proc_state = PROCESS_STATE_UNEXPECTED;
        }
    }
}

void process_run(void)
{
    unsigned int proc_idx;
    proc_entry_t *p_entry;

    // nothing to update
    if (proc_list.nr_entry == 0) {
        return;
    }

    proc_idx = find_first_bit(proc_list.assigned_entry_bitmap);
    while (proc_idx < MAX_PROCESS_CNT) {
        p_entry = &(proc_list.entry[proc_idx]);

        __update_process_result(&(proc_list.entry[proc_idx]));
        if (p_entry->proc_state == PROCESS_STATE_COMPLETE) {
            __free_process_entry(proc_idx);
        }

        proc_idx = find_next_bit(proc_list.assigned_entry_bitmap, proc_idx + 1);
    }

    exit(EXIT_SUCCESS);
}

unsigned int process_check_nr_entry(void)
{
    return proc_list.nr_entry;
}
