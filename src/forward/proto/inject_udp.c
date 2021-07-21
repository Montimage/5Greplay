/*
 * inject_udp.c
 *
 *  Created on: May 19, 2021
 *      Author: nhnghia
 *
 * This file implements the packet injection using UDP:
 * - Once a packet arrives, its UDP payload is forwarded using this implementation
 * So:
 * + work only with UDP packets
 * + this injector works as a UDP proxy
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <arpa/inet.h>

#include "inject_udp.h"
#include "../../lib/mmt_lib.h"

struct inject_udp_context_struct{
	int client_fd;
	uint16_t nb_copies;
	const char* host;
	uint16_t port;
};

void _udp_connect( inject_udp_context_t *context ){
	int conn_fd, ret;
	struct sockaddr_in servaddr = {
			.sin_family = AF_INET,
			.sin_port = htons( context->port ),
			.sin_addr.s_addr = inet_addr( context->host ),
	};

	conn_fd = socket(AF_INET, SOCK_DGRAM, 0);
	ASSERT( conn_fd >= 0, "Cannot create UDP socket, errno %d: %s", errno, strerror( errno ) );

	ret = connect(conn_fd, (struct sockaddr *) &servaddr, sizeof(servaddr));
	ASSERT( ret >= 0, "Cannot connect to %s:%d using UDP", context->host, context->port );

	context->client_fd = conn_fd;
}

inject_udp_context_t* inject_udp_alloc( const forward_packet_target_conf_t *conf, uint32_t nb_copies ){


	inject_udp_context_t *context = mmt_mem_alloc_and_init_zero( sizeof( struct inject_udp_context_struct ));
	context->host      = conf->host;
	context->port      = conf->port;
	context->nb_copies = nb_copies;
	_udp_connect( context );
	return context;
}

static inline void _clear_udp_buffer_if_need( inject_udp_context_t *context ){
	char buffer[1024];
	int ret;
	do {
		ret = recv( context->client_fd, buffer, sizeof( buffer), MSG_DONTWAIT );
		/*
		if (ret > 0) {
			printf("Received %d bytes data: %s\n", ret, buffer);
			fflush(stdout);
		}*/
	} while( ret > 0);
}

int inject_udp_send_packet( inject_udp_context_t *context, const uint8_t *packet_data, uint16_t packet_size ){
	uint16_t nb_pkt_sent = 0;
	int ret, i;

//	_reconnect_udp_if_need( context );
	//_clear_udp_buffer_if_need( context );

	for( i=0; i<context->nb_copies; i++ ){
		//returns the number of bytes written on success and -1 on failure.
		ret = send( context->client_fd, packet_data, packet_size, 0 );
		if( ret > 0 )
			nb_pkt_sent ++;
	}

	//sleep(1);
	return nb_pkt_sent;
}

void inject_udp_release( inject_udp_context_t *context ){
	if( !context )
		return;
	close(context->client_fd);

	mmt_mem_free( context );
}



