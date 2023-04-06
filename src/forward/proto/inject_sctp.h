/*
 * inject_sctp.h
 *
 *  Created on: May 14, 2021
 *      Author: nhnghia
 */

#ifndef SRC_MODULES_SECURITY_FORWARD_PROTO_INJECT_SCTP_H_
#define SRC_MODULES_SECURITY_FORWARD_PROTO_INJECT_SCTP_H_

#include "../../engine/configure.h"

typedef struct inject_sctp_context_struct inject_sctp_context_t;

inject_sctp_context_t* inject_sctp_alloc( const forward_packet_target_conf_t *conf, uint32_t nb_copies );
int inject_sctp_send_packet( inject_sctp_context_t *context, const uint8_t *packet_data, uint16_t packet_size );
void inject_sctp_release( inject_sctp_context_t *context );

void inject_sctp_update_param( inject_sctp_context_t *injector_context, uint32_t ppid, uint32_t flags, uint16_t stream_no, uint32_t timetolive, uint32_t ctx );

#endif /* SRC_MODULES_SECURITY_FORWARD_PROTO_INJECT_SCTP_H_ */
