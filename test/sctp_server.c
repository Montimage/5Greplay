/*
 * sctp_server.c
 *
 *  Created on: Oct 11, 2022
 *      Author: nhnghia
 *
 *  To compile: gcc sctp_server.c -o sctp_server -lsctp
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/sctp.h>

#define MAX_BUFFER 1024

int main(int argc, char* args[] ) {
	int listenSock, connSock, ret, in, flags, i;
	struct sockaddr_in servaddr;
	struct sctp_initmsg initmsg;
	struct sctp_event_subscribe events;
	struct sctp_sndrcvinfo sndrcvinfo;
	char buffer[MAX_BUFFER + 1];
	int port_number = 38412;

	if( argc == 2 )
		port_number = atoi( args[1] );

	listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);
	if (listenSock == -1) {
		printf("Failed to create socket\n");
		perror("socket()");
		exit(1);
	}

	bzero((void* ) &servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons( port_number );

	ret = bind(listenSock, (struct sockaddr*) &servaddr, sizeof(servaddr));

	if (ret == -1) {
		printf("Bind failed \n");
		perror("bind()");
		close(listenSock);
		exit(1);
	}

	/* Specify that a maximum of 5 streams will be available per socket */
	memset(&initmsg, 0, sizeof(initmsg));
	initmsg.sinit_num_ostreams = 5;
	initmsg.sinit_max_instreams = 5;
	initmsg.sinit_max_attempts = 4;
	ret = setsockopt(listenSock, IPPROTO_SCTP, SCTP_INITMSG, &initmsg,
			sizeof(initmsg));

	if (ret == -1) {
		printf("setsockopt() failed \n");
		perror("setsockopt()");
		close(listenSock);
		exit(1);
	}

	ret = listen(listenSock, 5);
	if (ret == -1) {
		printf("listen() failed \n");
		perror("listen()");
		close(listenSock);
		exit(1);
	}

	printf("SCTP server is listening on port %d ...\n", port_number );
	while (1) {

		char buffer[MAX_BUFFER + 1];
		int len;

		//Clear the buffer
		bzero(buffer, MAX_BUFFER + 1);

		printf("Awaiting a new connection\n");

		connSock = accept(listenSock, (struct sockaddr*) NULL, (int*) NULL);
		if (connSock == -1) {
			printf("accept() failed\n");
			perror("accept()");
			close(connSock);
			continue;
		} else
			printf("New client connected....\n");

		in = sctp_recvmsg(connSock, buffer, sizeof(buffer),
				(struct sockaddr*) NULL, 0, &sndrcvinfo, &flags);

		if (in == -1) {
			printf("Error in sctp_recvmsg\n");
			perror("sctp_recvmsg()");
			close(connSock);
			continue;
		} else {
			//Add '\0' in case of text data
			buffer[in] = '\0';

			printf(" Length of Data received: %d\n", in);
			printf(" Data : %s\n", (char*) buffer);
		}
		close(connSock);
	}

	return 0;
}
