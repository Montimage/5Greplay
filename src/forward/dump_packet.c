/*
 * dump_packet.c
 *
 *  Created on: Feb 10, 2022
 *      Author: nhnghia
 */


#include "dump_packet.h"
#include <sys/time.h>

// =======================> pcap file <=================================
struct pd_timeval {
	uint32_t tv_sec;     /* seconds */
	uint32_t tv_usec;    /* microseconds */
};

//see: https://wiki.wireshark.org/Development/LibpcapFileFormat
struct pd_pcap_file_header {
	uint32_t magic;
	uint16_t version_major;
	uint16_t version_minor;
	int32_t thiszone;     /* gmt to local correction */
	uint32_t sigfigs;     /* accuracy of timestamps */
	uint32_t snaplen;     /* max length saved portion of each pkt */
	uint32_t linktype;    /* data link type (LINKTYPE_*) */
};

struct pd_pcap_pkthdr {
	struct pd_timeval ts;  /* time stamp using 32 bits fields */
	uint32_t caplen;       /* length of portion present */
	uint32_t len;          /* length this packet (off wire) */
};



bool dump_packet_write_to_pcap_file( FILE *fd, const uint8_t* buf, uint16_t len){
	struct pd_pcap_pkthdr h;
	struct timeval tv;
	//get current time
	gettimeofday( & tv, NULL );

	h.ts.tv_sec  = (uint32_t)tv.tv_sec;
	h.ts.tv_usec = (uint32_t)tv.tv_usec;
	h.caplen = len;
	h.len    = len;

	//write header
	if( fwrite( &h, sizeof( h ), 1, fd ) != 1 )
		return false;

	//write packet data
	if( fwrite( buf, sizeof( uint8_t ), len, fd ) != len)
		return false;

	return true;
}

FILE * dump_packet_create_pcap_file(const char * path){
	//open file for writing
	FILE *file = fopen( path, "w" );
	if (file == NULL)
		return NULL;

	//write header of pcap file
	struct pd_pcap_file_header hdr;
	hdr.magic         = 0xa1b2c3d4; //
	hdr.version_major = 2;
	hdr.version_minor = 4;
	hdr.thiszone      = 0;
	hdr.snaplen       = 65355;
	hdr.sigfigs       = 0;
	hdr.linktype      = 1; //LINKTYPE_ETHERNET
	fwrite( &hdr, sizeof( hdr ), 1, file );

	return file;
}

void dump_packet_close_pcap_file( FILE *file ) {
	if( file )
		fclose( file );
}
