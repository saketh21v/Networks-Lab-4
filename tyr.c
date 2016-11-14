#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main(){
	int pid;
	char buf[256];
	pid = fork();
	if(pid == 0){
		execlp("/usr/bin/cvlc", "cvlc", "OS.mp3" , "&", (char*) NULL);
		exit(0);
	} else{
		printf("pid = %d\n", pid);
		// system("ps");
		char p;
		char cmd[256];
		char pidC[10];

		memset(cmd, '\0', 256);
		memset(pidC, '\0', 10);
		strcpy(cmd, "kill ");

		sprintf(pidC, "%d", pid);
		memcpy(&cmd[strlen("kill ")], pidC, strlen(pidC));
		printf("cmd : %s\n", cmd);
		
		p = getc(stdin);
		if(p == 'c'){
			system(cmd);
		}
	}
}