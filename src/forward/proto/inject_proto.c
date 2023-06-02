/*
 * inject_proto.c
 *
 *  Created on: May 14, 2021
 *      Author: nhnghia
 */

#include <mmt_core.h>
#include <tcpip/mmt_tcpip.h>
#include "../../lib/mmt_lib.h"
#include "inject_proto.h"

inject_proto_context_t* inject_proto_alloc( const config_t *config ){
	inject_proto_context_t *context = mmt_mem_alloc_and_init_zero( sizeof( inject_proto_context_t ));
	const forward_packet_conf_t *conf = config->forward;
	int i;
	const forward_packet_target_conf_t *target;

	for( i=0; i<conf->target_size; i++ ){
		target = & conf->targets[i];
		switch( target->protocol ){
		case FORWARD_PACKET_PROTO_SCTP:
			context->sctp = inject_sctp_alloc(target, conf->nb_copies );
			break;
		case FORWARD_PACKET_PROTO_UDP:
			context->udp = inject_udp_alloc(target, conf->nb_copies );
			break;

		case FORWARD_PACKET_PROTO_HTTP2:
			context->http2 = inject_http2_alloc(target, conf->nb_copies );
			break;

		default:
			ABORT("Does not support forwarding using a protocol to %s:%d", target->host, target->port );
		}
	}

	return context;
}

struct sctp_datahdr {
        uint8_t type;
        uint8_t flags;
        uint16_t length;
        uint32_t tsn;
        uint16_t stream;
        uint16_t ssn;
        uint32_t ppid;
        //uint8_t payload[0];
    };

//keep only SCTP payload in context->packet_data
static inline int _get_sctp_data_offset( const ipacket_t *ipacket ){
	int sctp_index = get_protocol_index_by_id( ipacket, PROTO_SCTP_DATA );
	//not found SCTP
	if( sctp_index == -1 )
		return -1;
	//offset of sctp in packet
	return get_packet_offset_at_index(ipacket, sctp_index) + sizeof( struct sctp_datahdr );
}


static inline int _get_udp_data_offset( const ipacket_t *ipacket ){
	int index = get_protocol_index_by_id( ipacket, PROTO_UDP );
	//not found SCTP
	if( index == -1 )
		return -1;
	//offset of sctp in packet
	return get_packet_offset_at_index(ipacket, index) + 8; //8 bytes of UDP header ( each 2 bytes: src, dst port, length, checksum)
}


static inline int _get_http2_data_offset( const ipacket_t *ipacket ){
	int index = get_protocol_index_by_id( ipacket, PROTO_HTTP2 );
	//not found Http
	if( index == -1 )
		return -1;
	//offset of sctp in packet
	return get_packet_offset_at_index(ipacket, index) ; 
}


int inject_proto_send_packet( inject_proto_context_t *context, const ipacket_t *ipacket, const uint8_t *packet_data, uint16_t packet_size ){
	int offset;
	int ret =  0, i;
	//when SCTP injector is enable
	if( context->sctp ){
		offset = _get_sctp_data_offset( ipacket );
		if( offset >= 0 ){
			DEBUG("%"PRIu64" SCTP_DATA offset: %d", ipacket->packet_id, offset );
			ret += inject_sctp_send_packet(context->sctp, packet_data + offset, packet_size - offset);
		}
	}
	if( context->udp ){
		offset = _get_udp_data_offset( ipacket );
		if( offset >= 0 ){
			DEBUG("Packet_id %"PRIu64" UDP_DATA offset: %d", ipacket->packet_id, offset );
			ret += inject_udp_send_packet(context->udp, packet_data + offset, packet_size - offset);
		}
	}
	if( context->http2 ){
		offset = _get_http2_data_offset( ipacket );
		//printf("%"PRIu64" HTTP2_DATA offset: %d\n", ipacket->packet_id,offset);
		if( offset >= 0 ){
			//printf("%"PRIu64" HTTP2_DATA offset: %d", ipacket->packet_id, offset );
			//HTTP2 is a specific protocol which will reject the duplicated data having the same stream id
			// we need to increase stream id each time sending one packet
			ret += inject_http2_send_packet(context->http2, packet_data + offset, packet_size-offset);
		}
	}

	if( ret == 0 )
		return INJECT_PROTO_NO_AVAIL;
	return ret;
}

void inject_proto_release( inject_proto_context_t *context ){
	if( context == NULL )
		return;
	inject_sctp_release(context->sctp);
	inject_udp_release(context->udp);
	inject_http2_release(context->http2);
	mmt_mem_free( context );
}
