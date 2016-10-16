#include "process.h"
#include <errno.h>

void terminate(char *line, proclist* list)
{
#if USE_GNU_READLINE == 1
	/* rl_clear_history() does not exist yet in centOS 6 */
	clear_history();
#endif
	if (line)
		free(line);
	/* We have to kill all our children before leaving */
	disp_jobs(list);
	kill_children(list);
	exit(0);
}

int special_calls(char* line,proclist* jobs_list)
{
	if (line == NULL || ! strncmp(line, "exit", 4))
		terminate(line, jobs_list);
	else if (! strncmp(line, "jobs", 4)){
		disp_jobs(jobs_list);
		return 1;
	}
	return 0;
}

void pipe_process(char*** seq)
{
	int pipe_tab[2];
	pipe(pipe_tab);
	int res = fork();
	// The son becomes the 'after pipe'
	if (res == 0) {
		dup2(pipe_tab[0], 0);
		close(pipe_tab[0]);
		close(pipe_tab[1]);
		execvp(*(seq[1]), seq[1]);
	// The grand son becomes the 'before pipe'
	} else {
		dup2(pipe_tab[1], 1);
		close(pipe_tab[0]);
		close(pipe_tab[1]);
		execvp(*(seq[0]), seq[0]);
	}
}

void redirect_process(struct cmdline* l)
{
	if (l->in) { // input redirection
		int in_fd = open(l->in, O_RDONLY); 
		dup2(in_fd, 0);
		close(in_fd);
	}
	if (l->out) { // output redirection
		int out_fd = open(l->out, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
		dup2(out_fd, 1);
		close(out_fd);
	}
}

void create_process(proclist* jobs_list, struct cmdline* l)
{
	uint32_t child_pid = fork();

	if (child_pid < 0) {
		fprintf(stderr,"Error when trying to fork.\n");
		exit(0);
	}

	// FATHER PROCESS
	if (child_pid != 0) {
		if (!l->bg)
			waitpid(child_pid,NULL, 0);
		else
			add(jobs_list, child_pid, l->seq[0]);
	} 

	// CHILD PROCESS
	else {
		// Redirect if needed
		redirect_process(l);
		// Pipe if needed
		if (l->seq[1] == NULL) {
			if (execvp(**(l->seq), *(l->seq)) == -1)
				exit(0);
		}
		else
			pipe_process(l->seq);
	}
}


int setup_line(struct cmdline** l, char* line, proclist* jobs_list)
{
	*l = parsecmd(&line);
	/* If input stream closed, normal termination */
	if (!(*l))
		terminate(0,jobs_list);
	if ((*l)->err) {
		/* Syntax error, read another command */
		printf("error: %s\n", (*l)->err);
		return 0;
	}
	if ((*l)->in) printf("in: %s\n", (*l)->in);
	if ((*l)->out) printf("out: %s\n", (*l)->out);
	if ((*l)->bg) printf("background (&)\n");
	return 1;
}
