/*
 * inject_udp.h
 *
 *  Created on: May 19, 2021
 *      Author: nhnghia
 */

#ifndef SRC_MODULES_SECURITY_FORWARD_PROTO_INJECT_UDP_H_
#define SRC_MODULES_SECURITY_FORWARD_PROTO_INJECT_UDP_H_

#include "../../engine/configure.h"


typedef struct inject_udp_context_struct inject_udp_context_t;

inject_udp_context_t* inject_udp_alloc( const forward_packet_target_conf_t *conf, uint32_t nb_copies );
int inject_udp_send_packet( inject_udp_context_t *context, const uint8_t *packet_data, uint16_t packet_size );
void inject_udp_release( inject_udp_context_t *context );

#endif /* SRC_MODULES_SECURITY_FORWARD_PROTO_INJECT_UDP_H_ */
