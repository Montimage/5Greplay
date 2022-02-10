/*
 * dump_packet.h
 *
 *  Created on: Feb 10, 2022
 *      Author: nhnghia
 */

#ifndef SRC_FORWARD_DUMP_PACKET_H_
#define SRC_FORWARD_DUMP_PACKET_H_

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include "../lib/mmt_lib.h"

FILE* dump_packet_create_pcap_file(const char * path);
bool  dump_packet_write_to_pcap_file( FILE *fd, const uint8_t* buf, uint16_t len );
void  dump_packet_close_pcap_file( FILE *file );

#endif /* SRC_FORWARD_DUMP_PACKET_H_ */
