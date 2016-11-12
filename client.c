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

#include "structures.h"

#define MC_PORT 5432
#define MCAST_ADDRESS "239.192.4.10"
#define SERVER_ADDRESS "10.6.4.246"
#define SERVER_PORT 12022
#define BUF_SIZE 4096
#define BUF_SIZE_SMALL 256

int TotalNoOfStations;
station_info stations[16];

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

int main(int argc, char * argv[]){

	receiveAndPrintStationList();

// 	int sD; /* UDP socket descriptor */
// 	struct sockaddr_in sin; /* socket struct */
// 	char *if_name; /* name of interface */
// 	struct ifreq ifr; /* interface struct */
// 	char buf[BUF_SIZE];
// 	int len;
// 	/* Multicast specific */
// 	char *mcast_addr; /* multicast address */
// 	struct ip_mreq mcast_req;	/* multicast join struct */
// 	struct sockaddr_in mcast_saddr; /* multicast sender*/
// 	socklen_t mcast_saddr_len;


// 	/* Add code to take port number from user */
// 	if ((argc==2)||(argc == 3)) {
// 		mcast_addr = argv[1];
// 	}
// 	else {
// 		fprintf(stderr, "usage:(sudo) receiver [interface_name (optional)]\n");
// 		exit(1);
// 	}

// 	if(argc == 2) {
// 		if_name = argv[1];
// 	}
// 	else
// 		if_name = "wlan0";


// 	/* create socket */
// 	if ((sD = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
// 		perror("receiver: socket");
// 		exit(1);
// 	}

// 	/* build address data structure */
// 	memset((char *)&sin, 0, sizeof(sin));
// 	sin.sin_family = AF_INET;
// 	sin.sin_addr.s_addr = htonl(INADDR_ANY);
// 	sin.sin_port = htons(MC_PORT);


// 	/*Use the interface specified */ 
// 	memset(&ifr, 0, sizeof(ifr));
// 	strncpy(ifr.ifr_name , if_name, sizeof(if_name)-1);

// 	if ((setsockopt(sD, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, 
// 		sizeof(ifr))) < 0)
// 	{
// 		perror("receiver: setsockopt() error");
// 		close(sD);
// 		exit(1);
// 	}

// 	/* bind the socket */
// 	if ((bind(sD, (struct sockaddr *) &sin, sizeof(sin))) < 0) {
// 		perror("receiver: bind()");
// 		exit(1);
// 	}

// 	/* Multicast specific code follows */

// 	/* build IGMP join message structure */
// 	mcast_req.imr_multiaddr.s_addr = inet_addr(mcast_addr);
// 	mcast_req.imr_interface.s_addr = htonl(INADDR_ANY);

// 	/* send multicast join message */
// 	if ((setsockopt(sD, IPPROTO_IP, IP_ADD_MEMBERSHIP, 
// 		(void*) &mcast_req, sizeof(mcast_req))) < 0) {
// 		perror("mcast join receive: setsockopt()");
// 	exit(1);
// }



// 	/* receive multicast messages */	
// printf("\nReady to listen!\n\n");

// while(1) {

// 		/* reset sender struct */
// 	memset(&mcast_saddr, 0, sizeof(mcast_saddr));
// 	mcast_saddr_len = sizeof(mcast_saddr);

// 		/* clear buffer and receive */
// 	memset(buf, 0, sizeof(buf));
// 	if ((len = recvfrom(sD, buf, BUF_SIZE, 0, (struct sockaddr*)&mcast_saddr, 
// 		&mcast_saddr_len)) < 0) {
// 		perror("receiver: recvfrom()");
// 	exit(1);
// }
// fputs(buf, stdout);

// 		/* Add code to send multicast leave request */

// }

// close(sD);
return 0;
}
