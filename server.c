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

#include "structures.h"

#define MC_PORT 5432
#define MC_SERVER 239.192.1.10
#define BUF_SIZE 4096
#define SERVER 10.6.4.246
#define SERVER_PORT 12022

#define NO_OF_STATIONS 2

station_info stations[NO_OF_STATIONS];

void fillStations(){ // Fill station infos in network byte format
  station_info si1;
  initStationInfo(&si1);
  si1.type = htons(13);
  si1.station_number = 1;//htons(1);
  si1.station_name_size = htonl(strlen("Station 1"));
  strcpy(si1.station_name, "Station 1");
  si1.data_port = htons(2332);

  memcpy(&stations[0], &si1, sizeof(station_info));

  station_info si2;
  initStationInfo(&si2);
  si2.type = htons(13);
  si2.station_number = 2;//htons(2);
  si2.station_name_size = htonl(strlen("Station 2"));
  strcpy(si2.station_name, "Station 2");
  si2.data_port = htons(2344);

  memcpy(&stations[1], &si2, sizeof(station_info));
}

void* startStationListServer(void* arg){
  struct sockaddr_in sin;
  int len;
  int s, new_s;
  char str[INET_ADDRSTRLEN];
  char *serverIP = "10.6.4.246";

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

void* startStation(void* arg){ // arg provides station_info, songs, and any other relevant info

}

int main(int argc, char * argv[]){
  //Initializing Stations
  fillStations();
  startStationListServer(NULL);

  int s; /* socket descriptor */
  struct sockaddr_in sin; /* socket struct */
  char buf[BUF_SIZE];
  int len;
  
  /* Multicast specific */
  char *mcast_addr; /* multicast address */
  

  /* Add code to take port number from user */
  if (argc==2) {
    mcast_addr = argv[1];
  }
  else {
    fprintf(stderr, "usage: sender multicast_address\n");
    exit(1);
  }
   
  /* Create a socket */
  if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("server: socket");
    exit(1);
  }
 
  /* build address data structure */
  memset((char *)&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = inet_addr(mcast_addr);
  sin.sin_port = htons(MC_PORT);

  printf("\nWrite messages below to multicast!\n\n");

  /* Send multicast messages */
  /* Warning: This implementation sends strings ONLY */
  /* You need to change it for sending A/V files */
  memset(buf, 0, sizeof(buf));
  
  while (fgets (buf, BUF_SIZE, stdin)) {
    if ((len = sendto(s, buf, sizeof(buf), 0,
		      (struct sockaddr *)&sin, 
		      sizeof(sin))) == -1) {
      perror("sender: sendto");
      exit(1);
    }
    
    memset(buf, 0, sizeof(buf));
  
  }
  
  close(s);  
  return 0;
}