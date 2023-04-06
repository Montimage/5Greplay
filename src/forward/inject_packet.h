/*
 * inject_packet.h
 *
 *  Created on: May 7, 2021
 *      Author: nhnghia
 *
 *  This file defines an interface to inject packets into a given NIC
 *  It will be then implemented either by using libpcap or DPDK
 */

#ifndef SRC_MODULES_SECURITY_FORWARD_INJECT_PACKET_H_
#define SRC_MODULES_SECURITY_FORWARD_INJECT_PACKET_H_

#include "../engine/configure.h"

typedef struct inject_packet_context_struct inject_packet_context_t;

/**
 * This is called only one at the beginning to allocate a context
 * @param config
 * @return
 */
inject_packet_context_t* inject_packet_alloc( const config_t *config );

/**
 * Send a packet to the output NIC
 * @param context
 * @param packet_data
 * @param packet_size
 * @return number of packets being successfully injected to the output NIC
 */
int inject_packet_send_packet( inject_packet_context_t *context, const uint8_t *packet_data, uint16_t packet_size );

/**
 * This is call only one at the end to release the context
 * @param context
 */
void inject_packet_release( inject_packet_context_t *context );

#endif /* SRC_MODULES_SECURITY_FORWARD_INJECT_PACKET_H_ */
