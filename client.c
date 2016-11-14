/* CSD 304 Computer Networks, Fall 2016
	 Lab 4, multicast receiver
	 Team: 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>

#include "structures.h"

#define MC_PORT 2301
#define MCAST_ADDRESS "239.192.4.10"
#define SERVER_ADDRESS "10.6.4.246"
#define SERVER_PORT 12022
#define BUF_SIZE 4096
#define BUF_SIZE_SMALL 256

int TotalNoOfStations;
station_info stations[16];

int changeTemp = 0;
int curVLCPid = 0;
int curStation = 0;

int argC;
int forceClose = 0;

pthread_t recvSongsPID;

void receiveAndPrintStationList(){
	int sT; // TCP Socket descriptor
	struct sockaddr_in serv_addr;
	
	if ((sT = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("receiver: socket");
		exit(1);
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);;
	serv_addr.sin_port = htons(SERVER_PORT);
	
	if (connect(sT,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
		perror("ERROR connecting");

	char buf[BUF_SIZE_SMALL];
	int rBytes = BUF_SIZE_SMALL;

	send(sT, "name.txt\n", 9, 0);

	uint32_t noOfStations = 0;
	
	read(sT, &noOfStations, sizeof(uint32_t));
	noOfStations = ntohl(noOfStations);
	TotalNoOfStations = noOfStations;

	printf("No. of Stations : %d\n", noOfStations);
	if(noOfStations > 16){
		printf("Too many stations!!!\n");
		exit(0);
	}

	int i=0;
	station_info* si = malloc(sizeof(station_info));

	for(i=0;i<noOfStations;i++){
		read(sT, si, sizeof(station_info));
		printf("Station No. %hu\n", si->station_number);
		printf("Station Name : %s\n", si->station_name);
		printf("Station M_CAST Port : %d\n\n", ntohs(si->data_port));

		memcpy(&stations[i], si, sizeof(station_info));
	}
	close(sT);
}

void* setupAndRecvSongs(void* args){

  char** argv = (char**)args;

  int s; /* socket descriptor */
  struct sockaddr_in sin; /* socket struct */
  char *if_name; /* name of interface */
  struct ifreq ifr; /* interface struct */
	char buf[BUF_SIZE];
	int len;
  /* Multicast specific */
  char *mcast_addr; /* multicast address */
  struct ip_mreq mcast_req;  /* multicast join struct */
  struct sockaddr_in mcast_saddr; /* multicast sender*/
	socklen_t mcast_saddr_len;

  /* Add code to take port number from user */
	if ((argC==2)||(argC == 3)) {
		mcast_addr = argv[1];
	}
	else {
		fprintf(stderr, "usage:(sudo) receiver multicast_address [interface_name (optional)]\n");
		exit(1);
	}

	if(argC == 3) {
		if_name = argv[2];
	}
	else
		if_name = "wlan0";


  /* create socket */
	if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("receiver: socket");
		exit(1);
	}

	//Setting port
	int mc_port;
	if(curStation > TotalNoOfStations){
		printf("No such station exists. Reverting to default station\n");
		mc_port = 2300;
		curStation = 0;
	}else {
		mc_port = ntohs(stations[curStation].data_port);
		printf("mc_port = %d\n", mc_port);
	}

  /* build address data structure */
	memset((char *)&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = htons(mc_port);


  /*Use the interface specified */ 
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name , if_name, sizeof(if_name)-1);

	if ((setsockopt(s, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, 
		sizeof(ifr))) < 0)
	{
		perror("receiver: setsockopt() error");
		close(s);
		exit(1);
	}

  /* bind the socket */
	if ((bind(s, (struct sockaddr *) &sin, sizeof(sin))) < 0) {
		perror("receiver: bind()");
		exit(1);
	}

  /* Multicast specific code follows */

  /* build IGMP join message structure */
	mcast_req.imr_multiaddr.s_addr = inet_addr(mcast_addr);
	mcast_req.imr_interface.s_addr = htonl(INADDR_ANY);

  /* send multicast join message */
	if ((setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, 
		(void*) &mcast_req, sizeof(mcast_req))) < 0) {
		perror("mcast join receive: setsockopt()");
		exit(1);
	}



  /* receive multicast messages */  
	printf("\nReady to listen!\n\n");  	

	FILE* fp;
	int counter = 0;

	printf("Whats up?\n");
	song_info* songInfo = (song_info*) malloc(sizeof(song_info));
	char* tempSongs[2];
	tempSongs[0] = "tempSong1.mp3";
	tempSongs[1] = "tempSong2.mp3";

	int changeTemp = 0;
	int cur = 0;

	char* tempSong = tempSongs[cur];
	while(1) {
		if(changeTemp){
			tempSong = tempSongs[1 ^ cur];
		}

	    /* reset sender struct */
		memset(&mcast_saddr, 0, sizeof(mcast_saddr));
		mcast_saddr_len = sizeof(mcast_saddr);

	    /* clear buffer and receive */
		memset(buf, 0, sizeof(buf));
		if ((len = recvfrom(s, buf, BUF_SIZE, 0, (struct sockaddr*)&mcast_saddr, 
			&mcast_saddr_len)) < 0) {
			perror("receiver: recvfrom()");
		exit(1);
	}

	uint8_t temp;
	if(len == sizeof(song_info)){
		printf("Len = %d. Checking if songInfo...\n", len);
		memcpy(songInfo, buf, len);
		printf("songInfo->type = %hu\n", songInfo->type);
		temp = (uint8_t)songInfo->type;

		if(temp == 12){
			printf("Current Song : %s\n", songInfo->song_name);
			printf("Next Song : %s\n", songInfo->next_song_name);
	        // fp = fopen(tempSong, "w");
	        // fclose(fp);
			continue;
		}
		else
			printf("Nope....\n");
		}

	    // fputs(buf, stdout);
	    // printf("Counter : %d , Len = %d, forceClose = %d\n", counter, len, forceClose);

	    if(counter++ == 10){
	    	curVLCPid = fork();
			if(curVLCPid == 0){
				execlp("/usr/bin/cvlc", "cvlc", tempSong, (char*) NULL);
			}
	    }

		fp = fopen(tempSong, "ab+");
		fwrite(buf, len, 1, fp);
		fclose(fp);

		    // write(fd, buf, len);

		    /* Add code to send multicast leave request */
		if(forceClose == 1)
			break;
	}
	close(s);
	forceClose = 0;
	fp = fopen(tempSong, "wb");
	fclose(fp);

	// printf("Exiting Thread\n");
	return NULL;
}

void runRadio(char* argv[]){
	pthread_create(&recvSongsPID, NULL, setupAndRecvSongs, argv);
}

void* checkAndCloseVLC(void* args){
	// system("ps > .psList");
	
	// FILE* fp = fopen(".psList", "r");
	// char line[256];
	char pidC[10];

	char cmd[256];

	memset(cmd, '\0', 256);
	memset(pidC, '\0', 10);

	strcpy(cmd, "kill ");

	sprintf(pidC, "%d", curVLCPid);
	memcpy(&cmd[strlen("kill ")], pidC, strlen(pidC));

	// while(!feof(fp)){
	// 	fscanf(fp, "%s\n", line);
	// 	if(strstr(line, pidC) != NULL){
			system(cmd);
			return NULL;
		// }
	// }

}

int main(int argc, char * argv[]){
	argC = argc;
	receiveAndPrintStationList();

	char o;
	char lo = 'r';
	runRadio(argv);
	int station;
	
	while(1){
		printf("r = run, p = pause, c = change station, e = exit\n");
		printf("Option : ");
		o = getc(stdin);
		if(o == 'e'){
			checkAndCloseVLC(NULL);
			forceClose = 1;
			exit(0);
		}
		else if(o == 'p' && lo != 'p'){
			printf("Pausing\n");
			checkAndCloseVLC(NULL);
			// pthread_cancel(recvSongsPID);
			forceClose = 1;
		}
		else if(o == 'r' && lo != 'r'){
			printf("Running\n");
			runRadio(argv);
		}
		else if(o == 'c' && lo != 'c'){
			checkAndCloseVLC(NULL);
			forceClose = 1;
			receiveAndPrintStationList();
			printf("Station : ");
			scanf("%d", &station);
			curStation = station - 1;
			runRadio(argv);
		}
		lo = o;
	}
	return 0;
}