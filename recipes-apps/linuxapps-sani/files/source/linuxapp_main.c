#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/include/common.h"
#include "ch2_normal_io/fileops.h"
#include "ch5_process_mgmt/process.h"

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

    printf("User input: <");
    for (int num_args = 1; num_args < argc; num_args++) {
        printf("\"%s\" ", argv[num_args]);
    }
    printf("> \n");

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

unsigned int handle_process(int argc, char **argv)
{
    char *p_command;
    int num_args;

    p_command = argv[2];

    // remove argv[0], argv[1], argv[2] (linuxapps, process, p_command)
    num_args = argc - 2;
    char *p_args[num_args];
    for (int arg_idx = 0; arg_idx < num_args; arg_idx++) {
        p_args[arg_idx] = argv[arg_idx + 2];
    }

    if (process_add(p_command, p_args, num_args) == FAILURE) {
        exit(EXIT_FAILURE);
    }

    while (process_check_nr_entry() != 0) {
        process_run();
    }

    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
    unsigned int ret;
    char strbuf[100] = "";

    ret = FAILURE;
    if (argc < 2) {
        goto usage;
    }

    if (!strcmp(argv[1], "fileops")) {
        ret = handle_fileops(argc, argv, strbuf);
    }
    else if (!strcmp(argv[1], "process")) {
        ret = handle_process(argc, argv);
    }

    return ret;

usage:
    printf("usage: <operation type> <cmd> <args1 args2 ...>\n");
    return ret;
}
