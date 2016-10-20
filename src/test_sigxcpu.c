#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>

int main(void) {
	struct timeval t1 = {0, 0};
	struct timeval t2 = {3, 0};
	struct itimerval t = {t1, t2};

	void encule (int s) {
		printf("MOTHERFUCKER\n");
	}

	struct sigaction sa;
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = encule;
	sigaction(SIGALRM, &sa, NULL);

	uint32_t child_pid = fork();

	// father
	if (child_pid != 0) {
		waitpid(child_pid, NULL, 0);
	// child
	} else {
		setitimer(ITIMER_REAL, &t, NULL);

		char* s[3];
		s[0] = "sleep";
		s[1] = "1000";
		s[2] = NULL;
		execvp(*s, s);
	}
}
