/*
 * forward.c
 *
 *  Created on: Jan 7, 2021
 *      Author: nhnghia
 */
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <time.h>

#include "forward_packet.h"
#include "inject_packet.h"
#include "proto/inject_proto.h"
#include "../lib/process_packet.h"
#include "../lib/mmt_lib.h"
#include "dump_packet.h"

#define MAX_PACKET_SIZE 0xFFFF

struct forward_packet_context_struct{
	const forward_packet_conf_t *config;
	uint64_t nb_forwarded_packets;
	uint64_t nb_dropped_packets;
	uint8_t *packet_data; //a copy of a packet
	uint16_t packet_size;
	uint16_t packet_delta; //used to shift packet data when forwarding
	bool has_a_satisfied_rule; //whether there exists a rule that satisfied
	const ipacket_t *ipacket;

	inject_packet_context_t *injector;    //injector by default
	inject_proto_context_t *proto_injector; //injector to inject a special protocol

	struct{
		uint32_t nb_packets, nb_bytes;
		time_t last_time;
	}stat;

	FILE *pcap_dump;
};

//TODO: need to be fixed in multi-threading
static forward_packet_context_t *cache = NULL;
static forward_packet_context_t * _get_current_context(){
	//TODO: need to be fixed in multi-threading
	//MUST_NOT_OCCUR( cache == NULL );
	return cache;
}


static inline void _update_stat( forward_packet_context_t *context, uint32_t nb_packets ){

	context->stat.nb_packets += nb_packets;
	context->stat.nb_bytes   += ( nb_packets * context->packet_size );
	time_t now = time(NULL); //return number of second from 1970
	if( now != context->stat.last_time ){
		float interval = (now - context->stat.last_time);
		log_write_dual(LOG_INFO, "Statistics of forwarded packets %.2f pps (total: %"PRIu64" packets), %.2f bps",
				context->stat.nb_packets   / interval,
				context->nb_forwarded_packets,
				context->stat.nb_bytes * 8 / interval);
		//reset stat
		context->stat.last_time  = now;
		context->stat.nb_bytes   = 0;
		context->stat.nb_packets = 0;
	}
}

static inline bool _send_packet_to_nic( forward_packet_context_t *context ){
	int ret = INJECT_PROTO_NO_AVAIL;
	uint16_t delta = context->packet_delta;
	const uint8_t *data = &context->packet_data[delta];
	const uint16_t size = context->packet_size - delta;
	//send the packet only if it has data to send
	if( delta >= context->packet_size )
		return false;

	if( context->config->is_enable ){
		//try firstly using a real connection by using proto_injector
		//do not use proto_injector when DPDK
		#ifndef NEED_DPDK
			ret = inject_proto_send_packet(context->proto_injector, context->ipacket, data, size);
		#endif
		//if no protocol is available, then use default injector (libpcap/DPDK) to inject the raw packet to output NIC
		if( ret == INJECT_PROTO_NO_AVAIL )
			ret = inject_packet_send_packet(context->injector,  data, size);

		if( ret > 0 ){
			context->nb_forwarded_packets += ret;
			_update_stat( context, ret );
		}
	}

	//dump to file
	if( context->pcap_dump )
		dump_packet_write_to_pcap_file( context->pcap_dump, data, size );
	return (ret > 0);
}

/**
 * Called only once to initial variables
 * @param config
 * @param dpi_handler
 * @return
 */
forward_packet_context_t* forward_packet_alloc( const config_t *config, mmt_handler_t *dpi_handler ){
	int i;
	const forward_packet_target_conf_t *target;
	const forward_packet_conf_t *conf = config->forward;

	forward_packet_context_t *context = mmt_mem_alloc_and_init_zero( sizeof( forward_packet_context_t ));
	context->config = conf;

	context->proto_injector = inject_proto_alloc(config);
	//init packet injector that can be PCAP or DPDK (or other?)
	context->injector = inject_packet_alloc(config);

	context->packet_data = mmt_mem_alloc( MAX_PACKET_SIZE ); //max size of a IP packet
	context->packet_size = 0;
	context->has_a_satisfied_rule = false;
	context->stat.last_time = time(NULL);

	if( config->dump_packet->is_enable ){
		context->pcap_dump = dump_packet_create_pcap_file( config->dump_packet->output_file );
		ASSERT(context->pcap_dump != NULL, "Cannot open '%s' file for writing  packets",
			config->dump_packet->output_file );
	}

	//TODO: not work in multi-threading
	cache = context;

	return context;
}

/**
 * Called only once to free variables
 * @param context
 */
void forward_packet_release( forward_packet_context_t *context ){
	if( !context )
		return;
	log_write_dual(LOG_INFO, "Number of packets being successfully forwarded: %"PRIu64", dropped: %"PRIu64,
			context->nb_forwarded_packets, context->nb_dropped_packets );
	if( context->injector ){
		inject_packet_release( context->injector );
		context->injector = NULL;
	}

	inject_proto_release( context->proto_injector );

	if( context->pcap_dump )
		dump_packet_close_pcap_file( context->pcap_dump );

	mmt_mem_free( context->packet_data );
	mmt_mem_free( context );
}

void forward_packet_mark_being_satisfied( forward_packet_context_t *context ){
	if( context == NULL )
		return;
	context->has_a_satisfied_rule = true;
}


/**
 * This function must be called on each coming packet
 *   but before any rule being processed on the the current packet
 */
void forward_packet_on_receiving_packet_before_rule_processing(const ipacket_t * ipacket, forward_packet_context_t *context){
	if( context == NULL )
		return;
	context->ipacket = ipacket;
	context->packet_size = ipacket->p_hdr->caplen;
	context->has_a_satisfied_rule = false;
	//copy packet data, then modify packet's content
	memcpy(context->packet_data, ipacket->data, context->packet_size );
}

/**
 *  This function must be called on each coming packet
 *   but after all rules being processed on the current packet
 */
void forward_packet_on_receiving_packet_after_rule_processing( const ipacket_t * ipacket, forward_packet_context_t *context ){
	if( context == NULL )
		return;
	//whether the current packet is handled by a engine rule ?
	// if yes, we do nothing
	if( context->has_a_satisfied_rule )
		return;
	if( context->config->default_action  == ACTION_DROP ){
		context->nb_dropped_packets ++;
	} else {
		if( ! _send_packet_to_nic(context) )
			context->nb_dropped_packets ++;
	}
}


/**
 * This function is called by mmt-engine when a FORWARD rule is satisfied
 *   and its if_satisfied="#drop"
 */
void mmt_do_not_forward_packet(){
	//do nothing
	forward_packet_context_t *context = _get_current_context();
	if( context == NULL )
		return;
	context->nb_dropped_packets ++;
}

/**
 * This function is called by mmt-engine when a FORWARD rule is satisfied
 *   and its if_satisfied="#update"
 *   or explicitly call forward() in an embedded function
 */
void mmt_forward_packet(){
	forward_packet_context_t *context = _get_current_context();
	if( context == NULL )
		return;
	_send_packet_to_nic(context);
}


//this function is implemented inside mmt-dpi to update NGAP protocol
extern uint32_t update_ngap_data( u_char *data, uint32_t data_size, const ipacket_t *ipacket, uint32_t proto_id, uint32_t att_id, uint64_t new_val );


//this function is implemented inside mmt-dpi to update HTTP2 protocol
extern int update_http2_data( u_char *data, uint32_t data_size, const ipacket_t *ipacket, uint32_t proto_id, uint32_t att_id, uint64_t new_val );

/**
 * This function is called by mmt-engine when a FORWARD rule is satisfied
 *   and its if_satisfied="#update( xx.yy, ..)"
 *   or explicitly call set_numeric_value in an embedded function
 */
void mmt_set_attribute_number_value(uint32_t proto_id, uint32_t att_id, uint64_t new_val){
	forward_packet_context_t *context = _get_current_context();
	if( context == NULL )
		return;
	int ret = 0;
	int difference;

	switch(proto_id){
	case(PROTO_NGAP):
		ret = update_ngap_data(context->packet_data, context->packet_size, context->ipacket, proto_id, att_id, new_val );
		if( ! ret )
			log_write( LOG_ERR, "Cannot set new value %"PRIu64" for att %d of proto %d for packet id %"PRIu64,
				new_val, att_id, proto_id, context->ipacket->packet_id);
		break;

	case(PROTO_HTTP2):
		difference = update_http2_data(context->packet_data, context->packet_size, context->ipacket, proto_id, att_id, new_val );

		// why 400?
		if( difference != 0 && difference < 400 && difference > -400 )//check that difference value is not too elevated or too small
			if( context->packet_size + difference >= 0 )
				context->packet_size = context->packet_size + difference;
			//printf("mmt_set_attribute_number_value difference %d \n",difference);
			//printf("mmt_set_attribute_number_value Packet size %d\n",context->packet_size);
	break;
	
	default:
		log_write( LOG_ERR, "Cannot set new value %"PRIu64" for att %d of proto %d for packet id %"PRIu64,
			new_val, att_id, proto_id, context->ipacket->packet_id);
			}

}


/**
 * This is an embedded function that can be called in rules by user
 *
 * Returns the offset in number of bytes from the beginning of the packet for the protocol at the given index
 * @param proto_id is the ID of the protocol to be replaced
 * @param data_length length of data
 * @param data a sequence of bytes that will set to protocol
 * @return the offset in number of bytes since the beginning of the packet of the protocol, if successfully.
 * Otherwise it returns a negative number:
 *   -1 if the proto_id does not exist in the packet
 *   -2 if data is too big
 */
int mmt_replace_data_at_protocol_id( uint32_t proto_id, uint16_t data_length, const char* data){
	forward_packet_context_t *context = _get_current_context();
	if( context == NULL )
		return -3;
	int index = get_protocol_index_by_id( context->ipacket, proto_id );
	//not found SCTP
	if( index == -1 )
		return -1;
	//offset of sctp in packet
	int offset = get_packet_offset_at_index( context->ipacket, index);
	int next_offset = get_packet_offset_at_index( context->ipacket, index+1);
	if( next_offset <= offset )
		next_offset = offset;
	if( next_offset >= context->packet_size )
		//TODO: a risk here when context->packet_size != context->ipacket->p_hdr->len
		next_offset = context->packet_size;
	int old_length = next_offset - offset;
	int new_packet_size = context->packet_size - old_length + data_length;

	if( new_packet_size > MAX_PACKET_SIZE )
		return -2;
	int backup_data_size = context->packet_size - next_offset;
	uint8_t tmp_data[ MAX_PACKET_SIZE ];
	//backup the data after the segment to be replaced
	memcpy( tmp_data, &context->packet_data[ next_offset ], backup_data_size );
	//replace the segment by the data
	memcpy( &context->packet_data[offset], data, data_length );
	//put the backup segment back to packet_data
	memcpy( &context->packet_data[offset+data_length], tmp_data, backup_data_size );
	//update the new size of data
	context->packet_size = new_packet_size;

	return offset;
}

/**
 *  This is an embedded function that can be called in rules by user
 *   to update the parameters in sctp_sendmsg function
 *  See : https://linux.die.net/man/3/sctp_sendmsg
 * @param ppid
 * @param flags
 * @param stream_no
 * @param timetolive
 * @return
 */
int mmt_update_sctp_param( uint32_t ppid, uint32_t flags, uint16_t stream_no, uint32_t timetolive ){
	forward_packet_context_t *context = _get_current_context();
	if( context == NULL )
		return -3;

	inject_sctp_update_param( context->proto_injector->sctp, ppid, flags, stream_no, timetolive, 0);
	return 0;
}
