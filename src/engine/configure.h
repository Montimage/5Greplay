/*
 * configure.h
 *
 *  Created on: Dec 12, 2017
 *          by: Huu Nghia
 */

#ifndef SRC_LIB_CONFIGURE_H_
#define SRC_LIB_CONFIGURE_H_

#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h> //for uint64_t PRIu64
#include <stdbool.h>
#include <errno.h>

typedef struct internet_service_address_struct{
	char *host_name;
	uint16_t port_number;
}internet_service_address_t;

typedef struct input_source_conf_struct{

	enum {ONLINE_ANALYSIS, OFFLINE_ANALYSIS} input_mode;
	enum {DPDK_CAPTURE, PCAP_CAPTURE} capture_mode;

	//input source for PCAP online mode (interface name) and for offline mode (pcap name), however for DPDK its interface port number
	char *input_source;
	uint16_t snap_len;
}input_source_conf_t;


typedef enum{
	CONF_IP_ENCAPSULATION_INDEX_FIRST   = 1,
	CONF_IP_ENCAPSULATION_INDEX_SECOND     ,
	CONF_IP_ENCAPSULATION_INDEX_THIRD      ,
	CONF_IP_ENCAPSULATION_INDEX_FOURTH     ,
	CONF_IP_ENCAPSULATION_INDEX_FIFTH      ,
	CONF_IP_ENCAPSULATION_INDEX_SIXTH      ,
	CONF_IP_ENCAPSULATION_INDEX_SEVENTH    ,
	CONF_IP_ENCAPSULATION_INDEX_EIGHTH     ,
	CONF_IP_ENCAPSULATION_INDEX_NINTH      ,
	CONF_IP_ENCAPSULATION_INDEX_TENTH      ,
	CONF_IP_ENCAPSULATION_INDEX_ELEVENTH   ,
	CONF_IP_ENCAPSULATION_INDEX_TWELFTH    ,
	CONF_IP_ENCAPSULATION_INDEX_THIRTEENTH ,
	CONF_IP_ENCAPSULATION_INDEX_FOURTEENTH ,
	CONF_IP_ENCAPSULATION_INDEX_FIFTEENTH  ,
	CONF_IP_ENCAPSULATION_INDEX_LAST   /*currently, MMT supports maximally 16 proto in hierarchies*/
}conf_ip_encapsulation_index_t;

typedef struct engine_conf_struct{
	uint16_t threads_size;
	char *excluded_rules;
	char *rules_mask;
	uint32_t max_instances;
	//indicate which IP will be analyzed in case of IP-in-IP
	conf_ip_encapsulation_index_t ip_encapsulation_index;
}engine_conf_t;


typedef struct output_conf_struct{
	bool is_enable;
	char *output_dir;
	uint16_t sample_interval;
	bool is_report_description;
}output_conf_t;

typedef enum {
	ACTION_FORWARD,
	ACTION_DROP
}forward_action_t;

typedef struct forward_packet_target_conf_struct{
	enum{ FORWARD_PACKET_PROTO_SCTP, FORWARD_PACKET_PROTO_UDP, FORWARD_PACKET_PROTO_HTTP2 } protocol;
	char * host;
	uint16_t port;
}forward_packet_target_conf_t;

typedef struct forward_packet_conf_struct{
	bool is_enable;
	char *output_nic;
	uint16_t snap_len;
	uint16_t promisc;
	uint32_t nb_copies;
	forward_action_t default_action;

	forward_packet_target_conf_t *targets;
	uint16_t target_size;
}forward_packet_conf_t;


typedef struct mempool_conf_struct{
	uint32_t max_bytes;
	uint16_t max_elements;
	uint16_t max_message_size;
	uint32_t smp_ring_size;
}mempool_conf_t;


typedef struct dump_packet_conf_struct{
	bool is_enable;
	char *output_file;
}dump_packet_conf_t;

/**
 * Configuration of MMT-Probe
 */
typedef struct config_struct{

	uint32_t stack_type; //dpi stack type
	char *dpdk_options;

	input_source_conf_t *input;
	output_conf_t *output;

	forward_packet_conf_t *forward;
	engine_conf_t *engine;
	mempool_conf_t *mempool;
	dump_packet_conf_t *dump_packet;
}config_t;


/**
 * Load configuration from a file
 * @param filename
 * @return
 */
config_t* conf_load_from_file( const char* filename );

/**
 * Free all memory allocated by @load_configuration_from_file
 * @param
 */
void conf_release( config_t * );


/**
 * Split a string into an array of string segments with separator is comma
 * @param string
 * @param proto_lst is a pointer to a the result array that will be created by the function.
 * @return number of segments in proto_lst
 * @note: user needs to free the segments in proto_lst and also proto_lst after using them
 */
size_t conf_parse_list( const char *string, char ***proto_lst );

/**
 * Validate a configuration to avoid any conflict among its parameters, such as,
 * if you is_enable http reconstruction, then tcp reassembly must be enabled also.
 * @param conf
 * @return
 */
int conf_validate( config_t *conf );

bool conf_parse_input_mode( int *result, const char *string );
bool conf_parse_forward_default_action(int *result, const char *value);

#endif /* SRC_LIB_CONFIGURE_H_ */
