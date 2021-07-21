/*
 * dpi_message_t.h
 *
 * Bridging the gap between mmt-dpi data and mmt-engine message
 *
 *  Created on: Mar 24, 2017
 *  Created by: Huu Nghia NGUYEN <huunghia.nguyen@montimage.com>
 */

#ifndef SRC_LIB_DPI_MESSAGE_T_H_
#define SRC_LIB_DPI_MESSAGE_T_H_


#include <mmt_core.h>
#include <tcpip/mmt_tcpip.h>

#include "message_t.h"

#ifndef ftp_command_struct
/**
 * FTP command structure: CMD PARAMETER
 */
typedef struct ftp_command_struct{
	uint16_t cmd;
	char *str_cmd;
	char *param;
}ftp_command_t;

/**
 * FTP response structure
 */
typedef struct ftp_response_struct{
	uint16_t code;
	char *str_code;
	char *value;
}ftp_response_t;

#endif

/**
 * Get length of payload of a protocol
 * @param ipacket
 * @param proto_id
 * @return
 */
static inline size_t dpi_get_payload_len(const ipacket_t * ipacket, uint32_t proto_id ){
	int  i = 0;
	uint16_t length = 0;
	uint16_t offset = 0;

	for (i = 1; i < ipacket->proto_hierarchy->len; i++){
		offset +=ipacket->proto_headers_offset->proto_path[i];

		if ( ipacket->proto_hierarchy->proto_path[i] == proto_id ){
			//get header offset of the proto after #proto_id
			if ( (i+1) < ipacket->proto_hierarchy->len){
				offset +=ipacket->proto_headers_offset->proto_path[i+1];
				length = ipacket->p_hdr->caplen - offset;

				return length;
			}
			return 0;
		}
	}

	return 0;
}

/**
 * Get length of data of a protocol
 * @param ipacket
 * @param proto_id
 * @return
 */
static inline size_t dpi_get_data_len( const ipacket_t * ipacket, uint32_t proto_id ){
	int  i = 0;

	uint16_t length = 0;
	uint16_t offset = 0;

	for (i = 1; i < ipacket->proto_hierarchy->len; i++){
		offset +=ipacket->proto_headers_offset->proto_path[i];
		if ( ipacket->proto_hierarchy->proto_path[i] == proto_id ){
			length = ipacket->p_hdr->caplen - offset;

			return length;
		}
	}
	return 0;
}

static inline size_t dpi_get_ip_option_len(const ipacket_t * ipacket ){
	uint8_t *ip_header_len = (uint8_t *) get_attribute_extracted_data( ipacket,PROTO_IP,IP_HEADER_LEN );

	if( likely( ip_header_len != NULL ))
		return  (*ip_header_len - 20); //IPv4 has 20 bytes of fixed header
	return 0;
}

/**
 * Public API
 * Convert data in format of MMT-Probe to data in format of MMT-Sec
 */
static inline int dpi_message_set_void_data( const ipacket_t *pkt, const void *data, message_t *msg, uint32_t proto_id, uint32_t att_id ){
	const void *new_data = NULL;
	size_t new_data_len  = 0;
	int new_data_type    = MMT_SEC_MSG_DATA_TYPE_BINARY;

	if( unlikely( data == NULL ))
		return 0;

	switch( att_id ){
	case PROTO_PAYLOAD:
		new_data_type = MMT_SEC_MSG_DATA_TYPE_BINARY;
		new_data      = data;
		new_data_len  = dpi_get_payload_len( pkt, proto_id );
		break;

	case PROTO_DATA:
		new_data_type = MMT_SEC_MSG_DATA_TYPE_BINARY;
		new_data      = data;
		new_data_len  = dpi_get_data_len( pkt, proto_id );
		break;

	case FTP_LAST_COMMAND:
		if ( proto_id == PROTO_FTP ){
			new_data_type = MMT_SEC_MSG_DATA_TYPE_STRING;
			new_data      = ((ftp_command_t *)data)->str_cmd;
			new_data_len  = strlen( ((ftp_command_t *)data)->str_cmd );
		}
		break;

	case FTP_LAST_RESPONSE_CODE:
		if ( proto_id == PROTO_FTP ){
			new_data_type = MMT_SEC_MSG_DATA_TYPE_STRING;
			new_data      = ((ftp_response_t *)data)->str_code;
			new_data_len  = strlen( ((ftp_response_t *)data)->str_code );
		}
		break;

	case IP_OPTS:
		if( proto_id == PROTO_IP){
			new_data_type = MMT_SEC_MSG_DATA_TYPE_BINARY;
			new_data      = data;
			new_data_len  = dpi_get_ip_option_len( pkt );
		}
		break;

	default:
		log_write( LOG_WARNING,"Need to process attribute %d.%d for packet %"PRIu64, proto_id, att_id, pkt->packet_id );
		break;
	}//end of switch( att_id )

	if( new_data_len == 0 || new_data == NULL )
		return 0;

	return set_element_data_message_t( msg, proto_id, att_id, new_data, new_data_type, new_data_len );
}


/**
 * Convert data encoded by mmt-dpi to one element of message_t.
 * - Input:
 * 	+ data    : data to be converted
 * 	+ type    : type of #data
 * - Output:
 * 	+ el  : element to be updated in message_t
 * 	+ msg : message containing el
 * - return:
 * 	+ 0 if success
 */
static inline int dpi_message_set_dpi_data( const void *data, int dpi_data_type, message_t *msg, uint32_t proto_id, uint32_t att_id ){
	double number       = 0;
	const void *new_data= NULL;
	size_t new_data_len = 0;
	int new_data_type   = MMT_SEC_MSG_DATA_TYPE_BINARY;

	//does not exist data for this proto_id and att_id
	if( unlikely( data == NULL ))
		return 1;

	switch( dpi_data_type ){
	case MMT_UNDEFINED_TYPE: /**< no type constant value */
		break;
	case MMT_DATA_CHAR: /**< 1 character constant value */
		number = *(char *) data;
		new_data_type = MMT_SEC_MSG_DATA_TYPE_NUMERIC;
		new_data      = &number;
		new_data_len  = sizeof( double );
		break;

	case MMT_U8_DATA: /**< unsigned 1-byte constant value */
		number    = *(uint8_t *) data;
		new_data_type = MMT_SEC_MSG_DATA_TYPE_NUMERIC;
		new_data      = &number;
		new_data_len  = sizeof( double );
		break;

	case MMT_DATA_PORT: /**< tcp/udp port constant value */
	case MMT_U16_DATA: /**< unsigned 2-bytes constant value */
		number    = *(uint16_t *) data;
		new_data_type = MMT_SEC_MSG_DATA_TYPE_NUMERIC;
		new_data      = &number;
		new_data_len  = sizeof( double );
		break;

	case MMT_U32_DATA: /**< unsigned 4-bytes constant value */
		number    = *(uint32_t *) data;
		new_data_type = MMT_SEC_MSG_DATA_TYPE_NUMERIC;
		new_data      = &number;
		new_data_len  = sizeof( double );
		break;

	case MMT_U64_DATA: /**< unsigned 8-bytes constant value */
		number    = *(uint64_t *) data;
		new_data_type = MMT_SEC_MSG_DATA_TYPE_NUMERIC;
		new_data      = &number;
		new_data_len  = sizeof( double );
		break;

	case MMT_DATA_FLOAT: /**< float constant value */
		number   =  *(float *) data;
		new_data_type = MMT_SEC_MSG_DATA_TYPE_NUMERIC;
		new_data      = &number;
		new_data_len  = sizeof( double );
		break;

	case MMT_DATA_IP6_ADDR: /**< ip6 address constant value */
		new_data_type = MMT_SEC_MSG_DATA_TYPE_BINARY;
		new_data      = data;
		new_data_len  = 16;
		break;

	case MMT_DATA_MAC_ADDR: /**< ethernet mac address constant value */
		new_data_type = MMT_SEC_MSG_DATA_TYPE_BINARY;
		new_data      = data;
		new_data_len  = 6;
		break;

	case MMT_DATA_IP_NET: /**< ip network address constant value */
	case MMT_DATA_IP_ADDR: /**< ip address constant value */
		new_data_type = MMT_SEC_MSG_DATA_TYPE_BINARY;
		new_data      = data;
		new_data_len  = 4;
		break;

	case MMT_DATA_TIMEVAL: /**< number of seconds and microseconds constant value */
		new_data_type = MMT_SEC_MSG_DATA_TYPE_BINARY;
		new_data      = data;
		new_data_len  = sizeof( struct timeval );
		break;

    //special cases: pointer is processed separately
	//case MMT_DATA_POINTER: /**< pointer constant value (size is void *) */

	case MMT_DATA_BUFFER: /**< binary buffer content */
	case MMT_DATA_POINT: /**< point constant value */
	case MMT_DATA_PORT_RANGE: /**< tcp/udp port range constant value */
	case MMT_DATA_DATE: /**< date constant value */
	case MMT_DATA_TIMEARG: /**< time argument constant value */
	case MMT_DATA_STRING_INDEX: /**< string index constant value (an association between a string and an integer) */
	case MMT_DATA_LAYERID: /**< Layer ID value */
	case MMT_DATA_FILTER_STATE: /**< (filter_id: filter_state) */
	case MMT_DATA_PARENT: /**< (filter_id: filter_state) */
	case MMT_STATS: /**< pointer to MMT Protocol statistics */
		printf("WARN: does not support MMT-DPI data type %d\n", dpi_data_type );
		break;

	case MMT_BINARY_DATA: /**< binary constant value */
		new_data_type = MMT_SEC_MSG_DATA_TYPE_BINARY;
		new_data      = data;
		new_data_len  = BINARY_64DATA_TYPE_LEN;
		break;

	case MMT_BINARY_VAR_DATA: /**< binary constant value with variable size given by function getExtractionDataSizeByProtocolAndFieldIds */
		new_data_type = MMT_SEC_MSG_DATA_TYPE_BINARY;
		new_data      = data;
		new_data_len  = BINARY_1024DATA_TYPE_LEN;
		break;

	case MMT_STRING_DATA: /**< text string data constant value. Len plus data. Data is expected to be '\0' terminated and maximum BINARY_64DATA_LEN long */
	case MMT_STRING_LONG_DATA: /**< text string data constant value. Len plus data. Data is expected to be '\0' terminated and maximum STRING_DATA_LEN long */
		new_data_type = MMT_SEC_MSG_DATA_TYPE_STRING;
		new_data      = ((mmt_binary_var_data_t *)data)->data;
		new_data_len  = ((mmt_binary_var_data_t *)data)->len;
		break;


	case MMT_HEADER_LINE: /**< string pointer value with a variable size. The string is not necessary null terminating */
		new_data_type = MMT_SEC_MSG_DATA_TYPE_STRING;
		new_data      = ((mmt_header_line_t *)data)->ptr;
		new_data_len  = ((mmt_header_line_t *)data)->len;
		break;

	case MMT_GENERIC_HEADER_LINE: /**< structure representing an RFC2822 header line with null terminating field and value elements. */
	case MMT_STRING_DATA_POINTER: /**< pointer constant value (size is void *). The data pointed to is of type string with null terminating character included */
		new_data_type = MMT_SEC_MSG_DATA_TYPE_STRING;
		new_data      = data;
		new_data_len  = strlen( (char*) data);
		break;

	case MMT_DATA_PATH: /**< protocol path constatn value */
		new_data_type = MMT_SEC_MSG_DATA_TYPE_BINARY;
		new_data      = data;
		new_data_len  = sizeof( proto_hierarchy_t );
		break;

	default:
		break;
	}

	if( unlikely( new_data_len == 0 || new_data == NULL ))
		return 0;

	return set_element_data_message_t( msg, proto_id, att_id, new_data, new_data_type, new_data_len );
}


/**
 * Convert data encoded in a pcap packet to readable data that is either a double
 * or a string ending by '\0'.
 * This function will create a new memory segment to store its result.
 */
static inline int dpi_message_set_data( const ipacket_t *pkt, int dpi_data_type, message_t *msg, uint32_t proto_id, uint32_t att_id ){
	uint8_t *data = (uint8_t *) get_attribute_extracted_data( pkt, proto_id, att_id );

	//does not exist data for this proto_id and att_id
	if( data == NULL )
		return 1;

	if( dpi_data_type == MMT_DATA_POINTER ){
		return dpi_message_set_void_data( pkt, data, msg, proto_id, att_id );
	}

	return dpi_message_set_dpi_data( data, dpi_data_type, msg, proto_id, att_id );
}

#endif /* SRC_LIB_DPI_MESSAGE_T_H_ */
