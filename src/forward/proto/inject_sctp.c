/*
 * sctp.c
 *
 *  Created on: May 12, 2021
 *      Author: nhnghia
 *
 * This file implements the packet injection using SCTP:
 * - Once a packet arrives, its SCTP payload is forwarded using this implementation
 * So:
 * + work only with SCTP packets
 * + this injector works as a SCTP proxy
 *
 * - Need "-lsctp" when compiling
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <arpa/inet.h>

#include "inject_sctp.h"
#include "../../lib/mmt_lib.h"

typedef struct sctp_param {
	uint32_t ppid;
	uint32_t flags;
	uint16_t stream_no;
	uint32_t timetolive;
	uint32_t context;
} sctp_param_t ;

struct inject_sctp_context_struct{
	int client_fd;
	uint16_t nb_copies;
	const char* host;
	uint16_t port;

	// the parameters to pass to sctp_sendmsg function to send sctp packets
	// see more: https://linux.die.net/man/3/sctp_sendmsg
	sctp_param_t sctp_param;
};

void _sctp_connect( inject_sctp_context_t *context ){
	int conn_fd, ret;
	struct sockaddr_in servaddr = {
			.sin_family = AF_INET,
			.sin_port = htons( context->port ),
			.sin_addr.s_addr = inet_addr( context->host ),
	};

	conn_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);
	ASSERT( conn_fd >= 0, "Cannot create SCTP socket" );

	ret = connect(conn_fd, (struct sockaddr *) &servaddr, sizeof(servaddr));
	ASSERT( ret >= 0, "Cannot connect to %s:%d using SCTP", context->host, context->port );

	context->client_fd = conn_fd;
}

inject_sctp_context_t* inject_sctp_alloc( const forward_packet_target_conf_t *conf, uint32_t nb_copies ){


	inject_sctp_context_t *context = mmt_mem_alloc_and_init_zero( sizeof( struct inject_sctp_context_struct ));
	context->host      = conf->host;
	context->port      = conf->port;
	context->nb_copies = nb_copies;
	//sctp parameters to send packets
	inject_sctp_update_param( context, 0,
			60, //payload protocol id of NGAP protcol
			0, 0, 0);

	_sctp_connect( context );
	return context;
}

void inject_sctp_update_param( inject_sctp_context_t *context, uint32_t ppid, uint32_t flags, uint16_t stream_no, uint32_t timetolive, uint32_t ctx ){
	context->sctp_param.context = ctx;
	context->sctp_param.flags   = flags;
	context->sctp_param.ppid    = htonl( ppid );
	context->sctp_param.stream_no = ( stream_no );
	context->sctp_param.timetolive = timetolive;
}

static inline void _reconnect_sctp_if_need( inject_sctp_context_t *context ){
	struct sctp_status status;
	int ret;
    int i = sizeof(status);
    ret = getsockopt( context->client_fd, SOL_SCTP, SCTP_STATUS, &status, (socklen_t *)&i);
    ASSERT( ret != 0, "Cannot get SCTP socket status");

    printf("\nSCTP Status:\n--------\n");
    printf("assoc id  = %d\n", status.sstat_assoc_id);
    printf("state     = %d\n", status.sstat_state);
    printf("instrms   = %d\n", status.sstat_instrms);
    printf("outstrms  = %d\n--------\n\n", status.sstat_outstrms);
}

static inline void _clear_sctp_buffer_if_need( inject_sctp_context_t *context ){
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

int inject_sctp_send_packet( inject_sctp_context_t *context, const uint8_t *packet_data, uint16_t packet_size ){
	uint16_t nb_pkt_sent = 0;
	int ret, i;

//	_reconnect_sctp_if_need( context );
	_clear_sctp_buffer_if_need( context );

	for( i=0; i<context->nb_copies; i++ ){
		//returns the number of bytes written on success and -1 on failure.
		ret = sctp_sendmsg( context->client_fd, packet_data,  packet_size, NULL,
				0,
				context->sctp_param.ppid, //payload protocol id
				context->sctp_param.flags,  //flags
				context->sctp_param.stream_no,  //stream no
				context->sctp_param.timetolive,  //TTL
				context->sctp_param.context   //context
			);
		//ret = sctp_send( context->client_fd, packet_data, packet_size, NULL, 0 );
		if( ret > 0 )
			nb_pkt_sent ++;
	}

	//sleep(1);
	return nb_pkt_sent;
}

void inject_sctp_release( inject_sctp_context_t *context ){
	if( !context )
		return;
	close(context->client_fd);

	mmt_mem_free( context );
}



