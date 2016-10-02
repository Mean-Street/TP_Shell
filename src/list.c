#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

proclist* create_list(void) {
	proclist* list = malloc(sizeof(proclist));
	list->head = NULL;
	list->tail = NULL;
	list->size = 0;
	return list;
}

void add(proclist* list, pid_t pid, char** command) {
	/* We store the command in one string */
	proc* child = malloc(sizeof(proc));
	uint32_t length = 0;
	for (uint32_t i=0; command[i] != NULL; i++)
		length += strlen(command[i])+1;
	child->command = calloc(length+1, 1);
	for (uint32_t i=0; command[i] != NULL; i++) {
		strcat(child->command, command[i]);
		strcat(child->command, " ");
	}

	child->pid = pid;
	child->next = NULL;
	child->running = true;

	if (list->head == NULL) {
		list->head = child;
		list->tail = child;
	} else {
		list->tail->next = child;
		list->tail = child;
	}
	list->size += 1;
}

void del(proclist* list, pid_t pid) {
	if (list->head == NULL) {
		fprintf(stderr, "Error : empty list.\n");
		return;
	}

	proc* prec = list->head;
	proc* curr = list->head;
	
	// if we remove the first element
	if (curr->pid == pid) {
		list->head = curr->next;
		free(curr->command);
		free(curr);
	}
	else {
		while (curr != NULL) {
			if (curr->pid != pid) {
				prec = curr;
				curr = curr->next;
			} else {
				prec->next = curr->next;
				// if we remove the last one
				if (curr->next == NULL) { 
					list->tail = prec;
				}
				free(curr->command);
				free(curr);
			}
		}
	}
	list->size -= 1;
}

void disp(proclist* list) {
	proc* child = list->head;
	char* state = NULL;
	for (uint32_t i=0; child != NULL;i++) {
		if (child->running)
			state = "Running";
		else
			state = "Done";
		printf("[%u]\t %s \t\t %u: %s\n",i,state,child->pid,child->command);
		child = child->next;
	}
}

void kill_children(proclist* list) {
	proc* child;
	while (list->head != NULL) {
		child = list->head;
		list->head = child->next;
		free(child->command);
		free(child);
	}
}
