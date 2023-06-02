/*
 * inject_TCP.h
 *
 *  Created on: May 19, 2021
 *      Author: nhnghia
 */

#ifndef SRC_MODULES_SECURITY_FORWARD_PROTO_INJECT_HTTP2_H_
#define SRC_MODULES_SECURITY_FORWARD_PROTO_INJECT_HTTP2_H_

#include "../../engine/configure.h"
#define HTTP2_PATH_FUZZ 9

typedef struct inject_tcp_context_struct inject_http2_context_t;

inject_http2_context_t* inject_http2_alloc( const forward_packet_target_conf_t *conf, uint32_t nb_copies );
int inject_http2_send_packet( inject_http2_context_t *context, const uint8_t *packet_data, uint16_t packet_size );
void inject_http2_release( inject_http2_context_t *context );

#endif /* SRC_MODULES_SECURITY_FORWARD_PROTO_INJECT_TCP_H_ */
