/*****************************************************
 * Copyright Grégory Mounié 2008-2015                *
 *           Simon Nieuviarts 2002-2009              *
 * This code is distributed under the GLPv3 licence. *
 * Ce code est distribué sous la licence GPLv3+.     *
 *****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "variante.h"
#include "readcmd.h"
#include "list.h"

proclist* jobs_list;

#ifndef VARIANTE
#error "Variante non défini !!"
#endif

/* Guile (1.8 and 2.0) is auto-detected by cmake */
/* To disable Scheme interpreter (Guile support), comment the
 * following lines.  You may also have to comment related pkg-config
 * lines in CMakeLists.txt.
 */

void terminate(char *line,proclist* list)
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
		if (l->seq[1] == NULL)
			execvp(**(l->seq), *(l->seq));
		else
			pipe_process(l->seq);
	}
}


/* Our handler will deal with multiple processes running in background */
void childhandler(int s)
{
	while (waitpid(-1,NULL,WNOHANG)>0);
}

int setup_line(struct cmdline** l, char* line)
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

#if USE_GUILE == 1
#include <libguile.h>

int question6_executer(char *line)
{
	struct cmdline* l;
	int res = setup_line(&l, line);
	if (res == 0)
		return 0;
	if (l->seq[0] != NULL) {
		create_process(jobs_list, l);
	}
	return 1;
}

SCM executer_wrapper(SCM x)
{
	printf("%p\n",jobs_list);
	return scm_from_int(question6_executer(scm_to_locale_stringn(x, 0)));
}
#endif



int main()
{

	/* Creating jobs list */
	jobs_list = create_list();

	printf("Variante %d: %s\n", VARIANTE, VARIANTE_STRING);

#if USE_GUILE == 1
	scm_init_guile();
	/* register "executer" function in scheme */
	scm_c_define_gsubr("executer", 1, 0, 0, executer_wrapper);
#endif

	/* Defining the signal handler */
	struct sigaction sa;
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = childhandler;
	sigaction(SIGCHLD,&sa,NULL);

	while (1) {
		struct cmdline *l;
		char *line=0;
		int res;
		char *prompt = "ensishell>";

		/* Readline use some internal memory structure that
		   can not be cleaned at the end of the program. Thus
		   one memory leak per command seems unavoidable yet */
		line = readline(prompt);
		if (line == NULL || ! strncmp(line, "exit", 4))
			terminate(line, jobs_list);
		else if (! strncmp(line, "jobs", 4))
			disp_jobs(jobs_list);

#if USE_GNU_READLINE == 1
		add_history(line);
#endif


#if USE_GUILE == 1
		/* The line is a scheme command */
		if (line[0] == '(') {
			char catchligne[strlen(line) + 256];
			sprintf(catchligne, "(catch #t (lambda () %s) (lambda (key . parameters) (display \"mauvaise expression/bug en scheme\n\")))", line);
			scm_eval_string(scm_from_locale_string(catchligne));
			free(line);
			continue;
		}
#endif

		/* parsecmd free line and set it up to 0 */
		res = setup_line(&l, line);
		if (res == 0)
			continue;
		if (l->seq[0] != NULL) {
			create_process(jobs_list, l);
		}
	}
}
