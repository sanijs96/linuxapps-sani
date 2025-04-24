#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/common/common.h"
#include "ch2_normal_io/file_ops.h"

unsigned int handle_fileops(int argc, char **argv, char *strbuf)
{
    unsigned int ret;
    const fileops_cmd_t *p_fileops_cmd;
    fileops_args_t args;

    char *p_cmdname;
    char *p_filename;
    unsigned int fileops_cmd_idx;

    if (argc < FILEOPS_NUM_ARGS_MIN) {
        goto usage;
    }

    fileops_cmd_idx = 0;
    p_cmdname = argv[2];
    p_filename = argv[3];

    for (; fileops_cmd_idx < FILEOPS_TYPE_MAX; fileops_cmd_idx++) {
        p_fileops_cmd = &fileops_cmd_args_list[fileops_cmd_idx];
        if (!strcmp(p_fileops_cmd->cmdname, p_cmdname)) {
            goto process_cmd;
        }
    }

    goto usage;

process_cmd:
    if (argc != p_fileops_cmd->num_args) {
        goto usage;
    }

    if (fileops_cmd_idx == FILEOPS_TYPE_CREATE) {
        args.create.filemode = atoi(argv[3]);
        ret = fileops_create(p_filename, &args);
    }
    else if (fileops_register(p_filename) == SUCCESS) {
        if (fileops_cmd_idx == FILEOPS_TYPE_READ) {
            args.read.length = atoi(argv[4]);
            args.read.buffer = strbuf;

            ret = fileops_read(p_filename, &args);
        }
        else if (fileops_cmd_idx == FILEOPS_TYPE_APPEND) {
            args.append.string = argv[4];
            ret = fileops_append(p_filename, &args);
        }
        else if (fileops_cmd_idx == FILEOPS_TYPE_REPLACE) {
            args.replace.old_string = argv[4];
            args.replace.new_string = argv[5];
            ret = fileops_replace(p_filename, &args);
        }
    }

    return fileops_release(p_filename);

usage:
    printf("usage: fileops <cmd> <args1 args2 ...>\n");
    return FAILURE;
}

int main(int argc, char **argv)
{
    unsigned int ret;
    char strbuf[100] = "";

    if (!strcmp(argv[1], "fileops")) {
        ret = handle_fileops(argc, argv, strbuf);
    }

    return ret;
}
