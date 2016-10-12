#ifndef list_h
#define list_h

#include <stdbool.h>
#include <stdint.h>

typedef struct child {
	pid_t pid;
	char* command;
	bool running;
	struct child* next;
} proc;

typedef struct {
	proc* head;
	proc* tail;
	uint32_t size;
} proclist;

proclist* create_list(void);
void add(proclist* list, pid_t pid, char** command);
void del(proclist* list, pid_t pid);
void disp_jobs(proclist* list);
void kill_children(proclist* list);
#endif
