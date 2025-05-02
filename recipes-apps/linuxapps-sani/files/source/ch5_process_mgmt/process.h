#ifndef __PROCESS_H__
#define __PROCESS_H__

#include "process_def.h"

unsigned int process_add(char *proc, char **args, int num_args);
unsigned int process_check_nr_entry(void);
void process_run(void);

#endif // __PROCESS_H__
