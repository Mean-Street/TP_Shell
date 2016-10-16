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
#include "process.h"

proclist* jobs_list;

#ifndef VARIANTE
#error "Variante non défini !!"
#endif

/* Guile (1.8 and 2.0) is auto-detected by cmake */
/* To disable Scheme interpreter (Guile support), comment the
 * following lines.  You may also have to comment related pkg-config
 * lines in CMakeLists.txt.
 */

/* Our handler will deal with multiple processes running in background */
void childhandler(int s)
{
	pid_t pid = 0;
	while ((pid = waitpid(-1,NULL,WNOHANG))>0)
		change_state(jobs_list,pid);
}


#if USE_GUILE == 1
#include <libguile.h>
#include "scheme.h"

int question6_executer(char *line)
{
	struct cmdline* l;
	if(setup_line(&l, line, jobs_list) == 0)
		return 0;
	if (l->seq[0] != NULL) {
		create_process(jobs_list, l);
		clean_list(jobs_list);
	}
	return 1;
}

SCM executer_wrapper(SCM x)
{
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

	char *line = NULL;
	char *prompt = "ensishell>";
	struct cmdline *l;

	while (1) {
		line = NULL;

		/* Readline use some internal memory structure that
		   can not be cleaned at the end of the program. Thus
		   one memory leak per command seems unavoidable yet */
		line = readline(prompt);
		
#if USE_GNU_READLINE == 1
		add_history(line);
#endif
		if (line == NULL || ! strncmp(line, "exit", 4)) {
			terminate(line, jobs_list);
		} else if (! strncmp(line, "jobs", 4)) {
			disp_jobs(jobs_list);
			continue;
		}

#if USE_GUILE == 1
		/* The line is a scheme command */
		if (line[0] == '(') {
			setup_scheme(line);
			continue;
		}
#endif
		if(setup_line(&l,line,jobs_list) == 0)
			continue;
		if (l->seq[0] != NULL) {
			create_process(jobs_list, l);
			clean_list(jobs_list);
		}
	}
}
