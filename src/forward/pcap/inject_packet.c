/*
 * inject_packet.c
 *
 *  Created on: May 7, 2021
 *      Author: nhnghia
 *
 * This file implements the packet injection using libpcap
 */

#include <pcap/pcap.h>

#include "../inject_packet.h"
#include "../../lib/mmt_lib.h"

struct inject_packet_context_struct{
	pcap_t *pcap_handler;
	uint16_t nb_copies;
};

/**
 * This is called only one at the beginning to allocate a context
 * @param config
 * @return
 */
inject_packet_context_t* inject_packet_alloc( const config_t *probe_config ){
	char pcap_errbuf[PCAP_ERRBUF_SIZE];
	pcap_errbuf[0] = '\0';

	const forward_packet_conf_t *conf = probe_config->forward;

	pcap_t* pcap = pcap_open_live( conf->output_nic, conf->snap_len,
			conf->promisc, //promisc mode
			0, //timeout
			pcap_errbuf );

	//having error?
	if (pcap_errbuf[0]!='\0')
		ABORT("Cannot open NIC %s to forward packets: %s", conf->output_nic, pcap_errbuf);

	inject_packet_context_t *context = mmt_mem_alloc_and_init_zero( sizeof( struct inject_packet_context_struct ));
	context->pcap_handler = pcap;
	context->nb_copies = conf->nb_copies;
	return context;
}

/**
 * Send a packet to the output NIC
 * @param context
 * @param packet_data
 * @param packet_size
 * @return number of packets being successfully injected to the output NIC
 */
int inject_packet_send_packet( inject_packet_context_t *context, const uint8_t *packet_data, uint16_t packet_size ){
	uint16_t nb_pkt_sent = 0;
	int ret, i;

	for( i=0; i<context->nb_copies; i++ ){
		//returns the number of bytes written on success and -1 on failure.
		ret = pcap_inject(context->pcap_handler, packet_data, packet_size );
		if( ret > 0 )
			nb_pkt_sent ++;
	}
	return nb_pkt_sent;
}

/**
 * This is call only one at the end to release the context
 * @param context
 */
void inject_packet_release( inject_packet_context_t *context ){
	if( !context )
		return;
	if( context->pcap_handler ){
		pcap_close(context->pcap_handler);
		context->pcap_handler = NULL;
	}

	mmt_mem_free( context );
}
