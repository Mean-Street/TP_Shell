#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include "readcmd.c"

// kill, getpid()
// Studies/IUT/SECOND/asr/yoon/Reseaux/TCP

void childhandler(int s){
	//while(waitpid(-1,NULL,WNOHANG)>0);
	while(wait(NULL));
}


int main(int argc,char** argv){
	struct sigaction sa;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	sa.sa_handler = childhandler;
	sigaction(SIGCHLD,&sa,NULL);

	if(fork()==0)
		execvp("/bin/ls",argv);
	else
		for(;;);
	return EXIT_SUCCESS;
}
