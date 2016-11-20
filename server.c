/* CSD 304 Computer Networks, Fall 2016
	 Lab 4, Sender
	 Team:
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

#include "structures.h"
#include "bitrate.h"

#define MC_PORT 2301
#define MC_SERVER 239.192.1.10
#define BUF_SIZE 4096
#define SERVER 10.6.4.246
#define SERVER_PORT 12022
#define BUF_SIZE_SMALL 1024

#define NO_OF_STATIONS 2

typedef struct station_id_path_t
{
	int id;
	char path[BUF_SIZE_SMALL];
	int port;
} station_id_path;

station_info stations[NO_OF_STATIONS];
station_id_path stationIDPaths[NO_OF_STATIONS];
pthread_t stationThreads[NO_OF_STATIONS];
long idealSleep;

void fillStations(){ // Fill station infos in network byte format
	station_info si1;
	station_id_path sip;

	initStationInfo(&si1);
	si1.type = 13;//htons(13);
	si1.station_number = 1;//htons(1);
	si1.station_name_size = htonl(strlen("Station 1"));
	strcpy(si1.station_name, "Station 1");
	si1.data_port = htons(2300);


	sip.port = 2300;
	sip.id = 1;
	strcpy(sip.path, "Station_1/");

	memcpy(&stations[0], &si1, sizeof(station_info));
	memcpy(&stationIDPaths[0], &sip, sizeof(station_id_path));

	bzero(&si1, sizeof(station_info));
	bzero(&sip, sizeof(station_id_path));

	initStationInfo(&si1);
	si1.type = 13;//htons(13);
	si1.station_number = 2;//htons(2);
	si1.station_name_size = htonl(strlen("Station 2"));
	strcpy(si1.station_name, "Station 2");
	si1.data_port = htons(2301);

	sip.port = 2301;
	sip.id = 2;
	strcpy(sip.path, "Station_2/");

	memcpy(&stations[1], &si1, sizeof(station_info));
	memcpy(&stationIDPaths[1], &sip, sizeof(station_id_path));
}

void* startStationListServer(void* arg){
	struct sockaddr_in sin;
	int len;
	int s, new_s;
	char str[INET_ADDRSTRLEN];
	// char *serverIP = "10.6.4.246";
	char *serverIP = "192.168.0.103";

	/* build address data structure */
	bzero((char *)&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = inet_addr(serverIP);;
	sin.sin_port = htons(SERVER_PORT);
	/* setup passive open */
	if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("simplex-talk: socket");
		exit(1);
	}

	inet_ntop(AF_INET, &(sin.sin_addr), str, INET_ADDRSTRLEN);
	printf("Server is using address %s and port %d.\n", str, SERVER_PORT);

	if ((bind(s, (struct sockaddr *)&sin, sizeof(sin))) < 0) {
		perror("simplex-talk: bind");
		exit(1);
	}
	else
		printf("Server bind done.\n");

	listen(s, 5);

	while(1){
		if ((new_s = accept(s, (struct sockaddr *)&sin, &len)) < 0) {
			perror("simplex-talk: accept");
			exit(1);
		}
		printf("Connected. Sending Structures\n");
		uint32_t qwe = 2;
		qwe = htonl(qwe);
		send(new_s, &qwe, sizeof(uint32_t), 0);

		for(int i=0;i<NO_OF_STATIONS;i++){
			send(new_s, &stations[i], sizeof(station_info), 0);
			printf("Send 1 structure\n");
		}
	}
}

void calcBitRate(char names[][BUF_SIZE_SMALL], int bitRate[], int songsCount){
	for(int i=0;i<songsCount;i++){
		bitRate[i] = getBitRate(names[i]);
	}
}

void* startStation(void* arg){
	//Parsing directory and opening songs
	station_id_path *sip = (station_id_path*)arg;
	DIR *dir;
	struct dirent *ent;
	int songsCount = 0;

	printf("Path : %s\n", sip->path);

	if ((dir = opendir (sip->path)) != NULL) {
		while ((ent = readdir (dir)) != NULL) {
				// printf ("%s\n", ent->d_name);
			if(strstr(ent->d_name, ".mp3") != NULL)
				++songsCount;
		}
		closedir (dir);
	} else {
		/* could not open directory */
		perror ("");
		return 0;
	}

	char songs[songsCount][BUF_SIZE_SMALL];
	char songNames[songsCount][BUF_SIZE_SMALL];

	FILE* songFiles[songsCount];
	int bitRates[songsCount];

	for(int i=0;i<songsCount;i++){
		memset(songs[i], '\0', BUF_SIZE_SMALL);
		strcpy(songs[i], sip->path);
	}

	int cur = 0;
	if ((dir = opendir (sip->path)) != NULL) {
		while ((ent = readdir (dir)) != NULL) {
				// printf ("%s\n", ent->d_name);
			if(strstr(ent->d_name, ".mp3") != NULL){
				memcpy(&(songs[cur][strlen(sip->path)]), ent->d_name, strlen(ent->d_name)+1);
				strcpy((songNames[cur]), ent->d_name);

				songFiles[cur] = fopen(songs[cur], "rb");
				if(songFiles[cur] == NULL){
					perror("");
					exit(1);
				}

				cur++;
			}
		}
		closedir (dir);
	}

	//Creating Song_Info Structures
	song_info songsInfo[songsCount];

	for(int i=0;i<songsCount;i++)
		bzero(&songsInfo[i], sizeof(song_info));

	for(int i=0;i<songsCount;i++){
		initSongInfo(&songsInfo[i]);
		printf("Init : %hu p = %p\n", songsInfo[i].type, &songsInfo[i].type);
	}

	for(int i=0;i<songsCount;i++){
		songsInfo[i].song_name_size = (uint8_t)strlen(songNames[i])+1;
		strcpy((songsInfo[i].song_name), songNames[i]);

		songsInfo[i].next_song_name_size = (uint8_t)strlen(songNames[(i+1)%songsCount]) + 1;
		strcpy((songsInfo[i].next_song_name), songNames[(i+1)%songsCount]);
	}

	//Calculating bitrates
	calcBitRate(songs, bitRates, songsCount);

	for(int i=0;i<songsCount;i++){
		printf("%s\n", songs[i]);
		printf("NONInit : %hu p = %p\n", songsInfo[i].type, &songsInfo[i].type);
	}
	////-------DONE Creating Files---------////


	////----Radio Stars here----////

	int s; /* socket descriptor */
	struct sockaddr_in sin; /* socket struct */
	int len;
	char buf[BUF_SIZE_SMALL];

	/* Multicast specific */
	char *mcast_addr; /* multicast address */

	struct timespec ts;
	ts.tv_sec = 0;
	// ts.tv_nsec = 200000000L;
	ts.tv_nsec = 20000000L;

	/* Add code to take port number from user */
	mcast_addr = "239.192.1.10";

	/* Create a socket */
	if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("server: socket");
		exit(1);
	}

	/* build address data structure */
	memset((char *)&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = inet_addr(mcast_addr);
	sin.sin_port = htons(sip->port);

	printf("\nStarting station ID : %d!\n\n", sip->id);

	/* Send multicast messages */
	/* Warning: This implementation sends strings ONLY */
	/* You need to change it for sending A/V files */
	memset(buf, 0, sizeof(buf));

	int curSong = -1;
	while (1) {
		curSong = (curSong + 1) % songsCount;

		FILE* song = songFiles[curSong];
		if(song == NULL) printf("FUCK YOU!!\n");
		printf("CurSong = %d Song = %p\n", curSong, song);
		rewind(song);

		int size = BUF_SIZE_SMALL;
		int counter = 0;
		printf("Sending Structure : curSong = %d. Song_Info->type = %hu p = %p\n", curSong, songsInfo[curSong].type, &songsInfo[curSong].type);

		if ((len = sendto(s, &songsInfo[curSong], sizeof(song_info), 0,(struct sockaddr *)&sin,sizeof(sin))) == -1) {
			perror("server: sendto");
			exit(1);
		}

		int bitrate = bitRates[curSong];

		//Calculating size of buffer
		// Calc time delay
		// idealSleep = ((BUF_SIZE_SMALL*8)/bitrate)*1000000;
		idealSleep = ((BUF_SIZE_SMALL*8)/bitrate)*500000000;

		if(idealSleep < 0)
			idealSleep = ts.tv_nsec;

		if(ts.tv_nsec > idealSleep)
			ts.tv_nsec = idealSleep;

		while(!(size < BUF_SIZE_SMALL)){
					// printf("Sending... T :%d Song: %d \n", sip->id,curSong);
			size = fread(buf, 1, sizeof(buf), song);

			if ((len = sendto(s, buf, size, 0,(struct sockaddr *)&sin,sizeof(sin))) == -1) {
				perror("server: sendto");
				exit(1);
			}
			if(len != size){
				printf("ERROR!!");
				exit(0);
			}
			// printf("Counter : %d , Size = %d , Len = %d\n", counter++, size, len);
			nanosleep(&ts, NULL);
			memset(buf, 0, sizeof(buf));
		}
	}

	close(s);
}

int main(int argc, char * argv[]){
	//Initializing Stations
	fillStations();

	// Starting TCP Server
	pthread_t tTCPid;
	pthread_create(&tTCPid, NULL, startStationListServer, NULL);
	// startStationListServer(NULL);

	//Starting all stations
	for(int i=0;i<NO_OF_STATIONS;i++){
	// int i = 0;
		pthread_create(&stationThreads[i], NULL, startStation, &stationIDPaths[i]);
		// startStation(&stationIDPaths[i]);
	}
	pthread_join(tTCPid, NULL);
	return 0;
}
