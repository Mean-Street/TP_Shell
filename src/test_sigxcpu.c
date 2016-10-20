#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>

int main(void) {
	struct rlimit t = {3, 8};

	uint32_t child_pid = fork();

	// father
	if (child_pid != 0) {
		waitpid(child_pid, NULL, 0);
	// child
	} else {
		setrlimit(RLIMIT_CPU, &t);
		char* s[3];
		s[0] = "sleep";
		s[1] = "1000";
		s[2] = NULL;
		/* while(1); */
		execvp(*s, s);
	}
}
