/*
 * message_t.c
 *
 *  Created on: Oct 20, 2016
 *  Created by: Huu Nghia NGUYEN <huunghia.nguyen@montimage.com>
 */

#include "message_t.h"
#include "expression.h"
#include "mmt_security.h"
#include "configure_override.h"

#define INIT_ID_VALUE 0
static mmt_memory_t *_memory = NULL;

static inline message_t* _create_local_message_t(){
	int i;
	const proto_attribute_t *const *proto_atts;
	message_t *msg;
	size_t _message_size;

	//create a reserved memory segment and initialize it
	//this is done only one time
	size_t elements_length = mmt_sec_get_unique_protocol_attributes( &proto_atts );

	size_t data_length =  conf_get_number_value( CONF_ATT__MEMPOOL__MAX_MESSAGE_SIZE );
	data_length  += elements_length * SIZE_OF_MMT_MEMORY_T;
	_message_size = sizeof( message_t )	//message
								+ sizeof( message_element_t) * elements_length //elements
								+ data_length //data
								;
	msg = mmt_mem_alloc( _message_size );
	EXEC_ONLY_IN_VALGRIND_MODE( ANNOTATE_HAPPENS_BEFORE( mmt_mem_revert( msg ) ) );
	//elements
	msg->elements_count = elements_length;

	msg->elements       = (message_element_t *) (&msg[1]); //store elements at the same date segment with msg

	//for each element
	for( i=0; i<msg->elements_count; i++ ){
		msg->elements[i].data     = NULL;
		msg->elements[i].proto_id = INIT_ID_VALUE;
		msg->elements[i].att_id   = INIT_ID_VALUE;
	}

	msg->hash         = 0;
	msg->_data_index  = 0;
	msg->_data        = &((uint8_t *) msg)[ sizeof( message_t ) + sizeof( message_element_t) * elements_length ];
	msg->_data_length = data_length;

	return msg;
}

message_t *create_message_t(){
	//clone the reserved memory
	message_t *msg;

	/**
	 * When allowing to add/rm rules at runtime => number of proto/atts will be change
	 * => size of message will be dynamic
	 */
#ifdef MODULE_ADD_OR_RM_RULES_RUNTIME
	return _create_local_message_t();
#else
	/**
	 * Using a memory segment to stock a template of message will increase the performance
	 * as we need to initialize messages only once
	 */
	if( unlikely( _memory == NULL ))
		_memory = mmt_mem_revert( _create_local_message_t() );

	//the message being created is a copy of _memory
	msg = mmt_mem_force_dup( _memory->data, _memory->size );
	EXEC_ONLY_IN_VALGRIND_MODE( ANNOTATE_HAPPENS_BEFORE( & msg ) );

	//update data pointers
	msg->elements = (message_element_t *)( msg + 1 );
	msg->_data    = &((uint8_t *) msg)[ sizeof( message_t ) + sizeof( message_element_t) * msg->elements_count ];
	msg->hash     = 0;
	return msg;
#endif
}


void force_free_message_t( message_t *msg ){
	mmt_mem_force_free( msg );
}

size_t free_message_ts( message_t *msg, uint16_t size ){
	size_t ret;
	__check_null( msg, 0 );  // nothing to do

	mmt_memory_t *mem = mmt_mem_revert( msg );

	//free message only when there is no more reference to it
	if( mem->ref_count <= size ){
		mmt_mem_force_free( msg );
		return 0;
	}

	ret = __sync_sub_and_fetch( &mem->ref_count, size );

	return ret;
}


message_element_t * get_element_message_t( const message_t *msg, uint32_t proto_id, uint32_t att_id ){
	int index = mmt_sec_hash_proto_attribute( proto_id, att_id );

#ifdef DEBUG_MODE
	if( unlikely( index >= msg->elements_count )){
		log_write( LOG_ERR,"Access to outside message's elements (%d > %d)", index, msg->elements_count);
		return NULL;
	}
#endif

	return &msg->elements[ index ];
}

/*
const void *get_element_data_message_t( const message_t *msg, uint16_t index ){

#ifdef DEBUG_MODE
	if( unlikely( index >= msg->elements_count )){
		log_write( LOG_ERR,"Access to outside message's elements");
		return NULL;
	}
#endif

	return msg->elements[ index ].data;
}
*/


int set_element_data_message_t( message_t *msg, uint32_t proto_id, uint32_t att_id, const void *data, enum data_type data_type, size_t data_length ){
	mmt_memory_t *mem;
	message_element_t *el;
	int index;

	//check if enough room to stock data
	if( unlikely (msg->_data_index + data_length + SIZE_OF_MMT_MEMORY_T + 1 >= msg->_data_length )){
		log_write( LOG_WARNING, "Report %"PRIu64" for %d.%d is too big (req. %zu, avail. %d bytes), must increase \"%s\"",
				msg->counter,
				proto_id, att_id, data_length + SIZE_OF_MMT_MEMORY_T, msg->_data_length - msg->_data_index,
				conf_get_name_from_id( CONF_ATT__MEMPOOL__MAX_MESSAGE_SIZE ));
		return MSG_OVERFLOW;
	}
	//do not need NULL
	else if( unlikely( data_length == 0 || data == NULL )){
		return MSG_CONTINUE;
	}
	//special filter for:
	//- proto_id = TCP
	//- att_id = FLAGS, FIN, SYN, RST, PSH, ACK, URG, ECE, CWR
	//=> if its data is zero => obmit => consider as NULL
	else if( proto_id == 354 && data_type == MMT_SEC_MSG_DATA_TYPE_NUMERIC && (*(double*) data)  == 0 && (6 <= att_id && att_id <= 14 ))
		return MSG_CONTINUE;


	index = mmt_sec_hash_proto_attribute( proto_id, att_id );

#ifdef DEBUG_MODE
	if( unlikely( index >= msg->elements_count )){
		log_write( LOG_ERR,"Access to outside message's elements (%d > %d)", index, msg->elements_count);
		return MSG_CONTINUE;
	}
#endif

	//update hash to mark the present of elem->data
	BIT_SET( msg->hash, index );

	el = & msg->elements[ index ];

	el->proto_id  = proto_id;
	el->att_id    = att_id;
	el->data_type = data_type;

	//convert to mmt_memory_t
	mem = (mmt_memory_t *) &msg->_data[ msg->_data_index ];
	mmt_mem_reset( mem, data_length );
	el->data = mem->data;

	memcpy( el->data, data, data_length );

	msg->_data_index += data_length + SIZE_OF_MMT_MEMORY_T + 1;

	return MSG_CONTINUE;
}

/**
 * This function is automatically called when finishing mmt-sec
 */
__attribute__((destructor)) void reset_message_t () {
	if( _memory != NULL ){
		mmt_mem_free( _memory->data );
		_memory = NULL;
	}
}
