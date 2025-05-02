#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>

#include "../common/include/common.h"
#include "fileops_def.h"
#include "fileops.h"

static filelist_t filelist = {0, };
static fd_set write_fd = {0, };
static fd_set read_fd = {0, };

const fileops_cmd_t fileops_cmd_args_list[FILEOPS_TYPE_MAX] = {
    [FILEOPS_TYPE_CREATE]   = {"create", FILEOPS_NUM_ARGS_CREATE},
    [FILEOPS_TYPE_REGISTER] = {"register", FILEOPS_NUM_ARGS_REGISTER},
    [FILEOPS_TYPE_READ]     = {"read", FILEOPS_NUM_ARGS_READ},
    [FILEOPS_TYPE_APPEND]   = {"append", FILEOPS_NUM_ARGS_APPEND},
    [FILEOPS_TYPE_REPLACE]  = {"replace", FILEOPS_NUM_ARGS_REPLACE},
};

static unsigned int __search_filelist(const char *filename, filelist_t *p_filelist)
{
    unsigned int entry_idx;
    filelist_entry_t *p_entry;

    entry_idx = 0;
    p_entry = &(p_filelist->entry[0]);

    while (entry_idx < MAX_FILELIST_ENTRY) {
        if (!strncmp(filename, p_entry->filename, MAX_FILENAME_LENGTH)) {
            break;
        }

        p_entry++;
        entry_idx++;
    }

    return entry_idx;
}

static unsigned int __search_free_filelist_entry(filelist_t *p_filelist)
{
    unsigned int entry_idx;
    filelist_entry_t *p_entry;

    entry_idx = 0;
    p_entry = &(p_filelist->entry[0]);
    while (entry_idx < MAX_FILELIST_ENTRY) {
        if (p_entry->fd == 0) {
            break;
        }

        p_entry++;
    }

    return entry_idx;
}

static int __handle_exception_create(int errno)
{
    return FAILURE;
}

unsigned int fileops_create(const char *filename, fileops_args_t *p_args)
{
    int oflag;

    oflag = p_args->create.filemode;

    return __handle_file_ops(creat(filename, oflag),
                                __handle_exception_create(errno));
}

static int __handle_exception_open(int errno)
{
    return FAILURE;
}

static int __register_to_filelist(const char *filename, unsigned int idx, filelist_t *p_filelist)
{
    int fd;
    filelist_entry_t *p_entry;

    p_entry = &p_filelist->entry[idx];

    fd = __handle_file_ops((p_entry->fd = open(filename, O_RDWR)),
                            __handle_exception_open(errno));

    if (fd != -1) {
        strncpy(p_entry->filename, filename, MAX_FILENAME_LENGTH); 

        p_filelist->nr_entry++;
    }

    return fd;
}

unsigned int fileops_register(const char *filename)
{
    unsigned int ret;
    unsigned int entry_idx;

    ret = SUCCESS;
    if (filelist.nr_entry == MAX_FILELIST_ENTRY) {
        printf("[register] filelist entry is full\n");
        ret = FAILURE;
        goto exit;
    }

    entry_idx = __search_filelist(filename, &filelist);
    if (entry_idx < MAX_FILELIST_ENTRY) {
        printf("[register] file already registered: %u\n", entry_idx);
        goto exit;
    }

    entry_idx = __search_free_filelist_entry(&filelist);

    ret = __register_to_filelist(filename, entry_idx, &filelist);
    if (ret == FAILURE) {
        printf("[register] failed with error %d\n", errno);
    }

exit:
    return ret;
}

static int __handle_exception_read(int errno)
{
    return FAILURE;
}

static unsigned int __read_from_file(filelist_entry_t *p_entry, char *buf, size_t len)
{
    FD_ZERO(&read_fd);
    FD_SET(p_entry->fd, &read_fd);

    // using multiplexed i/o
    select(p_entry->fd + 1, &read_fd, NULL, NULL, NULL);

    return __handle_file_ops(read(p_entry->fd, buf, len),
                                __handle_exception_read(errno));
}

unsigned int fileops_read(const char *filename, fileops_args_t *p_args)
{
    unsigned int ret;
    unsigned int entry_idx;
    filelist_entry_t *p_entry;

    entry_idx = __search_filelist(filename, &filelist);
    if (entry_idx == MAX_FILELIST_ENTRY) {
        printf("[read] file not registered\n");

        ret = FAILURE;
        goto exit;
    }

    p_entry = &filelist.entry[entry_idx];

    lseek(p_entry->fd, 0, SEEK_SET);

    ret = __read_from_file(p_entry, p_args->read.buffer, p_args->read.length);
    if (ret == FAILURE) {
        printf("[read] failed with error %d\n", errno);
    }
    else {
        printf("[read] result: %s\n", &p_args->read.buffer[0]);
    }

exit:
    return ret;
}

static int __handle_exception_write(int errno)
{
    return FAILURE;
}

static unsigned int __write_to_file(filelist_entry_t *p_entry, char *buf)
{
    FD_ZERO(&write_fd);
    FD_SET(p_entry->fd, &write_fd);

    // using multiplexed i/o
    select(p_entry->fd + 1, NULL, &write_fd, NULL, NULL);

    return __handle_file_ops(write(p_entry->fd, buf, strlen(buf)),
                            __handle_exception_write(errno));
}

unsigned int fileops_append(const char *filename, fileops_args_t *p_args)
{
    unsigned int ret;
    unsigned int entry_idx;
    filelist_entry_t *p_entry;

    entry_idx = __search_filelist(filename, &filelist);
    if (entry_idx == MAX_FILELIST_ENTRY) {
        printf("[write] file not registered\n");

        ret = FAILURE;

        goto exit;
    }

    p_entry = &(filelist.entry[entry_idx]);

    lseek(p_entry->fd, 0, SEEK_END);

    ret = __write_to_file(p_entry, p_args->append.string);
    if (ret == SUCCESS) {
        fdatasync(p_entry->fd);
    }
    else {
        printf("[write] failed with error %d\n", errno);
    }

exit:
    return ret;
}

static int __move_position_to_string(char *ref_strbuf, filelist_entry_t *p_entry)
{
    int res;
    int filepos;
    int file_total_len;
    const int strbuf_window_size = 100;
    char src_strbuf[strbuf_window_size];
    const char *p_strpos;

    res = SUCCESS;

    file_total_len = lseek(p_entry->fd, 0, SEEK_END);

    //initial condition
    lseek(p_entry->fd, 0, SEEK_SET);
    filepos = 0;

    while ((file_total_len > 0) &&
            (__read_from_file(p_entry, src_strbuf, strbuf_window_size) == SUCCESS)) {
        p_strpos = strstr(src_strbuf, ref_strbuf);
        if (p_strpos != NULL) { // hit string
            break;
        }

        filepos += strbuf_window_size;
        file_total_len -= strbuf_window_size;
    }

    if (p_strpos == NULL) {
        res = FAILURE;
    }
    else {
        filepos += (int)(p_strpos - src_strbuf);
        lseek(p_entry->fd, filepos, SEEK_SET);
    }

    return res;
}

unsigned int fileops_replace(const char *filename, fileops_args_t *p_args)
{
    int ret;
    unsigned int entry_idx;
    filelist_entry_t *p_entry;
    ret = SUCCESS;

    entry_idx = __search_filelist(filename, &filelist);
    if (entry_idx == MAX_FILELIST_ENTRY) {
        printf("[replace] file not registered\n");

        ret = FAILURE;
        goto exit;
    }

    p_entry = &(filelist.entry[entry_idx]);

    if (__move_position_to_string(p_args->replace.old_string, p_entry) == FAILURE) {
        printf("[replace] string not found, ignore & return\n");

        ret = SUCCESS;

        goto exit;
    }

    ret = __write_to_file(p_entry, p_args->replace.new_string);
    if (ret == SUCCESS) {
        fdatasync(p_entry->fd);
    }

exit:
    return ret;
}

static int __handle_exception_close(int errno)
{
    return FAILURE;
}

static void __delete_from_filelist(filelist_entry_t *p_entry, filelist_t *p_filelist)
{
    if (FD_ISSET(p_entry->fd, &read_fd)) {
        FD_CLR(p_entry->fd, &read_fd);
    }

    if (FD_ISSET(p_entry->fd, &write_fd)) {
        FD_CLR(p_entry->fd, &write_fd);
    }

    p_entry->fd = 0;
    memset(p_entry->filename, 0, MAX_FILENAME_LENGTH);

    p_filelist->nr_entry--;

exit:
    return;
}


unsigned int fileops_release(const char *filename)
{
    unsigned int ret;
    unsigned int entry_idx;
    filelist_entry_t *p_entry;

    ret = SUCCESS;

    entry_idx = __search_filelist(filename, &filelist);
    if (entry_idx == MAX_FILELIST_ENTRY) {
        printf("[release] file not registered\n");

        goto exit;
    }

    p_entry = &filelist.entry[entry_idx];

    __delete_from_filelist(p_entry, &filelist);

    ret = __handle_file_ops(close(p_entry->fd),
                            __handle_exception_close(errno));

    if (ret == FAILURE) {
        printf("[release] failed with error %d", errno);
    }

exit:
    return ret;
}
