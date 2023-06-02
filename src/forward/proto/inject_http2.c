/*
 * inject_tcp.c
 *
 *  Created on: May 19, 2023
 *      Author: francesco
 *
 * This file implements the packet injection using tcp:
 * - Once a packet arrives, its tcp payload is forwarded using this implementation
 * So:
 * + work only with tcp packets
 * + this injector works as a tcp proxy
 *
 *  02 Juin 2023 by HN
 *  - correct indentation
 *  - reconnect to server if being unconnected
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include "inject_http2.h"
#include "../../lib/mmt_lib.h"

struct inject_tcp_context_struct{
	int client_fd;
	uint16_t nb_copies;
	const char* host;
	uint16_t port;
	size_t total_sent_pkt;
	size_t total_pkt_to_send;
};

static void _http2_handshake(inject_http2_context_t *context){
	//Establish http2 connection
	int ret;	
	// Send the SETTINGS frame to the server
	const char magic_settings[] = {
			0x50,0x52,0x49,0x20,0x2a,0x20,0x48,0x54,0x54,0x50,0x2f,0x32,0x2e,0x30,0x0d,0x0a,0x0d,0x0a,0x53,0x4d,0x0d,0x0a,0x0d,0x0a,//preface HTTP2
			0x00,0x00,0x1e,//Settings length
			0x04,//type 
			0x00,0x00,0x00,0x00,0x00,
			0x00,0x03,0x00,0x00,0x00,0x64,//Max concurrent streams
			0x00,0x04,0x00,0x00,0xff,0xff,//initial window size
			0x00,0x01,0x00,0x00,0x10,0x00,//header table size
			0x00,0x02,0x00,0x00,0x00,0x00,//Ebable push
			0x00,0x06,0x00,0x00,0x07,0xd0//Max header list size

	};
	ret = send(context->client_fd, magic_settings, sizeof(magic_settings), 0);
	if( ret < 0) 
		log_write( LOG_ERR, "Cannot send HTTP2 SETTINGS FRAME to %s:%d", context->host, context->port );
	// Wait for the server's response
	char response[4096];
	memset(response, 0, sizeof(response));
	ret = read(context->client_fd, response, sizeof(response));
	if( ret <0)
		log_write( LOG_ERR, "Cannot receive HTTP2 ANSWER from %s:%d", context->host, context->port );

	//printf("I received this response %s with \n",response);
	const char settings_0[] = {
			0x00,0x00,0x00,//length
			0x04,//setting
			0x01,0x00,0x00,0x00,0x00 //flags
	};
	ret = send( context->client_fd, settings_0, sizeof(settings_0), 0);
	if( ret < 0)
		log_write( LOG_ERR, "Cannot send HTTP2 SETTINGS_0 frame from %s:%d", context->host, context->port );
	//printf("I sent settings[0] frame   \n");
}

void _http2_connect( inject_http2_context_t *context ){
	int conn_fd, ret;

	struct sockaddr_in servaddr = {
			.sin_family = AF_INET,
			.sin_port = htons( context->port ),
			.sin_addr.s_addr = inet_addr( context->host ),
	};

	conn_fd = socket(AF_INET, SOCK_STREAM,0);
	if( conn_fd < 0){
		log_write(conn_fd >= 0, "Cannot create TCP socket: (%d) %s", errno, strerror( errno ) );
		//cannot do anything else if no socket ==> exit
		abort();
	}
	// Set TCP No Delay option
	int flag = 1;
	int result = setsockopt(conn_fd, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int));
	if (result < 0) {
		log_write(LOG_ERR, "Error setting TCP No Delay option");
	}

	do {
		//log_write( LOG_INFO, "Connecting to %s:%d using HTTP2", context->host, context->port );
		ret = connect(conn_fd, (struct sockaddr *) &servaddr, sizeof(servaddr));
		if( ret < 0){
			log_write( LOG_ERR, "Cannot connect to %s:%d using TCP: (%d) %s. Try to reconnect in 10 seconds",
					context->host, context->port, errno, strerror(errno) );
			sleep(10);
		}
	} while( ret == -1 );

	log_write( LOG_INFO, "Connected to %s:%d using TCP", context->host, context->port );
	context->client_fd = conn_fd;
	// realize handshake when TCP socket can connect to the remote server
	_http2_handshake( context );
}

inject_http2_context_t* inject_http2_alloc( const forward_packet_target_conf_t *conf, uint32_t nb_copies ){


	inject_http2_context_t *context = mmt_mem_alloc_and_init_zero( sizeof( struct inject_tcp_context_struct ));
	context->host      = conf->host;
	context->port      = conf->port;

	//TODO: currently HTTP2 injector supports injecting only one packet at the time
	// more duplicated packet will be rejected by the server which raises GOAWAY message, then closes the socket.
	// A work around is to modify STREAM ID to get different packets
	//  This modification is done in the .xml rules concerning HTTP2 protocol, e.g., rules 13, 14, .., 17
	// The nb_copies will be transfered to the rules via MMT_5GREPLAY_HTTP2_NB_COPIES environment variable
	context->nb_copies = 1; //nb_copies;
	_http2_connect( context );
	return context;
}

static inline void _clear_tcp_buffer_if_need( inject_http2_context_t *context ){
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

int inject_http2_send_packet( inject_http2_context_t *context, const uint8_t *packet_data, uint16_t packet_size ){
	uint16_t nb_pkt_sent = 0;
	int ret, i;

	_clear_tcp_buffer_if_need( context );
	context->total_pkt_to_send++;

	for( i=0; i<context->nb_copies; i++ ){
		//returns the number of bytes written on success and -1 on failure.
		ret = send( context->client_fd, packet_data, packet_size, 0 );

		if( ret > 0 ){
			nb_pkt_sent ++;
			//printf("Http2 packet sent \n");
			context->total_sent_pkt ++;
			//clear the reception buffer for each X sending packets
			if( nb_pkt_sent % 10 == 0 )
				_clear_tcp_buffer_if_need( context );
		}
		//ret=read(context->client_fd, response, sizeof(response));
		//if( ret < 0)
		//	printf( "Cannot receive Http2 Answer from %s:%d using tcp\n", context->host, context->port );
		//reconnect if need
		if( ret == -1 ){
			log_write( LOG_ERR, "Cannot inject %d-th copy of %zu-th packet size %d to %s:%d using HTTP2: (%d) %s",
					(i+1), context->total_pkt_to_send, packet_size,
					context->host, context->port, errno, strerror(errno) );
			close(context->client_fd);
			_http2_connect( context );
		}
	}
	//sleep(1);
	return nb_pkt_sent;
}

void inject_http2_release( inject_http2_context_t *context ){
	if( !context )
		return;
	close(context->client_fd);

	mmt_mem_free( context );
}



