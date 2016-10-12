#include <stdlib.h>
#include <stdio.h>
#include "list.h"
#include "readcmd.h"
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>


#ifndef PROCESS_H
#define PROCESS_H

void terminate(char *line, proclist* list);

void pipe_process(char*** seq);

void redirect_process(struct cmdline* l);

void create_process(proclist* jobs_list, struct cmdline* l);

int setup_line(struct cmdline** l, char* line, proclist* jobs_list);
#endif
