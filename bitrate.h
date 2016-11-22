#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>


int getBitRate(char *fName){
	int pid = 0;
	char *cmd1 = "mediainfo ";
	char *redirect = " | grep 'Overall bit rate                         : ' > info.txt";
	char cmd[256];
	strcpy(cmd, cmd1);
	strcat(cmd, fName);
	strcat(cmd, redirect);
	system(cmd);
		// execl(params[0], params[1], params[2],params[3],NULL);
	wait(NULL);
	FILE* fp = fopen("info.txt", "r");
	char *str = "Overall bit rate                         : ";
	char buf[256];
	memset(buf, '\0', sizeof(buf));
	char *s;
	fgets(buf, sizeof(buf), fp);
	// printf("buf = %s\n", buf);
	char* p = strstr(buf,"Kbps\n");
	// printf("p = %s\n", p);
	memset(p, '\0', strlen(p));

	s = strstr(buf,str);
	s = s+strlen(str);
	
	char buf1[256];
	memset(buf1, '\0', 256);

	int i=0,j=0;
	for(i=0;i<strlen(s)+1;i++){
		if(s[i] == ' ')
			continue;
		buf1[j++] = s[i];
	}

	int br = atoi(buf1);
	return br;
	// printf("%d\n", br);
}

