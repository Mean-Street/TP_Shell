#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "readcmd.c"

// kill, getpid()
// Studies/IUT/SECOND/asr/yoon/Reseaux/TCP

void childhandler(int s){
	while(waitpid(-1,NULL,WNOHANG)>0);
}

int main(int argc,char** argv){
	struct sigaction sa;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	sa.sa_handler = childhandler;
	//sigaction(SIGCHLD,&sa,NULL);
	pid_t pid = 1;

	for(uint8_t i=0;i<10;i++){
		pid = fork();
		if(pid == 0){
			sleep(10-i);
			return 0;
		}
		else {
			waitpid(pid,NULL,WNOHANG);
			//;
		}
	}
	for(;;)
		system("ps");
	return EXIT_SUCCESS;
}
