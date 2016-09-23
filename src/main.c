#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include "readcmd.c"

// kill, getpid()
// Studies/IUT/SECOND/asr/yoon/Reseaux/TCP

void chldhandler(int s){
	//while(waitpid(-1,NULL,WNOHANG)>0);
	while(wait(NULL));
}

int main(int argc,char** argv){
	struct sigaction sa;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	sa.sa_handler = chldhandler;
	sigaction(SIGCHLD,&sa,NULL);

	char* prompt = ">";
	printf("%s\n",readline(prompt));
	return EXIT_SUCCESS;
}
