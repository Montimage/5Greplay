/*
 * message_t.h
 *
 *  Created on: Oct 20, 2016
 *  Created by: Huu Nghia NGUYEN <huunghia.nguyen@montimage.com>
 */

#ifndef SRC_LIB_MESSAGE_T_H_
#define SRC_LIB_MESSAGE_T_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>
#include "../lib/mmt_lib.h"


enum data_type{
	MMT_SEC_MSG_DATA_TYPE_NUMERIC,
	MMT_SEC_MSG_DATA_TYPE_STRING,
	MMT_SEC_MSG_DATA_TYPE_BINARY
};

/**
 *
 */
typedef struct message_element_struct{
	uint32_t proto_id;
	uint32_t att_id;
	int data_type; //MMT_SEC_MSG_DATA_TYPE_NUMERIC, MMT_SEC_MSG_DATA_TYPE_STRING, MMT_SEC_MSG_DATA_TYPE_BINARY
	void *data;
}message_element_t;


typedef struct message_struct{
	uint64_t counter;
	uint64_t timestamp;
	uint64_t flow_id;

	message_element_t *elements;
	//number of total elements being allocated
	uint16_t elements_count;

	//a hash number represents the present of its elements
	// bit i-th of the number is 1 if data of i-th element is not null,
	// otherwise, the data is null
	uint64_t hash;

	//for internal usage
	uint8_t *_data;
	uint32_t _data_index; //index of data
	uint32_t _data_length;

	//user_data allows users to store the context when giving the message to MMT-Sec
	// and get the context when having alerts
	void *user_data;
} __aligned message_t;

#define MSG_CONTINUE 0
#define MSG_OVERFLOW 1
#define MSG_DROP     2

message_t *create_message_t();

message_t *clone_message_t( const message_t msg, bool is_copy_data );

void force_free_message_t( message_t *msg );

/**
 * Free a message_t
 * This function reduces the number of references of #msg.
 * If there does not exist any more any references to #msg, then
 * its resource will be freed.
 *
 * One can increase number of references of a variable by using either
 * #mmt_mem_retain or #mmt_mem_retains
 */
size_t free_message_ts( message_t *msg, uint16_t size );

static inline size_t free_message_t( message_t *msg ){
	return free_message_ts( msg, 1);
}

/**
 * Copy data to elem->data
 * @param msg
 * @param proto_id
 * @param att_id
 * @param data
 * @param data_type
 * @param data_length
 * @param length
 * @return
 *		- MSG_CONTINUE if data is successfully put to #msg
 *		- MSG_OVERFLOW if #msg has no more place to store #data
 *		- MSG_DROP     if engine is not interested on the rest of #msg.
 *		                  It occurs when #msg contains enough information.
 *		                  Thus mmt-probe should stop extracting the other proto.att
 */
int set_element_data_message_t( message_t *msg, uint32_t proto_id, uint32_t att_id, const void *data, enum data_type data_type, size_t data_length );

/**
 * Get one element of message by proto_id and att_id
 * @param msg
 * @param proto_id
 * @param att_id
 * @return
 */
message_element_t *get_element_message_t( const message_t *msg, uint32_t proto_id, uint32_t att_id );


//const void *get_element_data_message_t( const message_t *msg, uint16_t index );

static inline const void *get_element_data_message_t( const message_t *msg, uint16_t index ){

#ifdef DEBUG_MODE
	if( unlikely( index >= msg->elements_count )){
		log_write( LOG_ERR,"Access to outside message's elements (%d > %d)", index, msg->elements_count);
		return NULL;
	}
#endif

	return msg->elements[ index ].data;
}

/**
 * This function is used to update message structure to take in to account some modification of rules set.
 * The modification leads to change elements size of messages.
 *
 * This function must be called from the one being calling #create_message_t
 */
void reset_message_t();


#endif /* SRC_LIB_MESSAGE_T_H_ */
