/*
 * engine.h
 *
 *  Created on: Mar 17, 2017
 *  Created by: Huu Nghia NGUYEN <huunghia.nguyen@montimage.com>
 */

#ifndef SRC_LIB_MMT_SECURITY_H_
#define SRC_LIB_MMT_SECURITY_H_

#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <sys/time.h>

//#include "mmt_lib.h"
#include "plugin_header.h"
#include "configure_override.h"
#include "../lib/version.h"

typedef struct mmt_sec_handler_struct mmt_sec_handler_t;

enum verdict_type {VERDICT_DETECTED, VERDICT_NOT_DETECTED, VERDICT_RESPECTED, VERDICT_NOT_RESPECTED, VERDICT_UNKNOWN};
static const char* verdict_type_string[] = {"detected", "not_detected", "respected", "not_respected", "unknown"};

/**
 * A function to be called when a rule is validated
 */
typedef void (*mmt_sec_callback)(
		const rule_info_t *rule,		//rule being validated
		enum verdict_type verdict,		//DETECTED, NOT_RESPECTED
		uint64_t timestamp,  			//moment (by time) the rule is validated
		uint64_t counter,					//moment (by order of packet) the rule is validated
		const mmt_array_t * const trace,//historic of messages that validates the rule
		void *user_data					//#user-data being given in mmt_sec_register_rules
		);


/**
 * This function init globally mmt-engine
 * It must be called from main thread before any register_security
 * @return
 */
int mmt_sec_init( const char *excluded_rules_id );

/**
 * This function closes globally mmt-engine
 * It must be called from the thread that called #mmt_sec_init
 */
void mmt_sec_close( );


/**
 * This function create a new group consisting of several threads to process a set of rules.
 * - Input
 * 	+ threads_count: number of threads
 * 	+ core_mask    : a string indicating logical cores to be used,
 * 						  e.g., "1-8,11-12,19" => we use cores 1,2,..,8,11,12,19
 *    + rule_mask    : a string indicating special rules being attributed to special threads
 *    						e.g., "(1:10-13)(2:50)(4:1007-1010)"
 *    						The other rules will be attributed equally to the rest of threads.
 * 	+ callback     : a function to be called when a rules is validated
 * 	+ user_data    : data will be passed to the #callback
 * - Return a handler pointer
 * - Note:
 * 	The function callback can be called from different threads. (Thus if it accesses
 * 	to a global variable or a static one, the access to these variables must be synchronous)
 */
mmt_sec_handler_t* mmt_sec_register( size_t threads_count, const uint32_t *cores_id, const char *rules_mask,
		bool verbose, mmt_sec_callback callback, void *user_data );

/**
 * Ignore the remain of a flow when an alert was detected on it.
 * This function must not be called after #mmt_sec_process
 * @param handler
 * @param ignore
 * @param buffer_size is the size of buffer to remember the IDs of flows having alerts.
 */
void mmt_sec_set_ignore_remain_flow( mmt_sec_handler_t *handler, bool ignore, uint64_t buffer_size );


/**
 * Check whether the remain of a flow can be ignored.
 * The rest of a flow can be ignored from verification if we have the 2 following conditions:
 * - the parameter ignore is true when calling #mmt_sec_set_ignore_remain_flow
 * - found an alert on the flow
 *
 * @param handler
 * @param flow_id
 * @return
 */
bool mmt_sec_is_ignore_remain_flow( mmt_sec_handler_t *handler, uint64_t flow_id );

/**
 * Give message to validate
 */
void mmt_sec_process( mmt_sec_handler_t *handler, message_t *msg );

/**
 * Stop and free engine handler
 * @param
 * @return number of alerts being generated
 */
size_t mmt_sec_unregister( mmt_sec_handler_t* );

/**
 * init mmt-engine engine:
 * - load plugins (encoded rules)
 */
size_t mmt_sec_get_rules_info( rule_info_t const*const**rules_array );


/**
 * Get list of unique protocols and their attributes needed by the given #handler
 */
size_t mmt_sec_get_unique_protocol_attributes( proto_attribute_t const *const **proto_atts_array );

/**
 * Return an unique number representing the pair proto_id and att_id
 * @param proto_id
 * @param att_id
 * @return
 */
uint16_t mmt_sec_hash_proto_attribute( uint32_t proto_id, uint32_t att_id );

/**
 * Encode a #timeval to an uint64_t value
 */
static inline uint64_t mmt_sec_encode_timeval( const struct timeval *t ){
        uint64_t val = t->tv_sec;
        return val * CLOCKS_PER_SEC  + t->tv_usec;;
}

/**
 * Decode an uint64_t value to a #timeval
 */
static inline void mmt_sec_decode_timeval( uint64_t val, struct timeval *time ){
        time->tv_sec  = val / CLOCKS_PER_SEC;     //timestamp: second
        time->tv_usec = val % CLOCKS_PER_SEC ; //timestamp: microsecond
}

const char* mmt_convert_execution_trace_to_json_string( const mmt_array_t *trace, const rule_info_t *rule );


/**
 * Print information of the rules existing.
 */
void mmt_sec_print_rules_info();


/**
 * Remove a set of rules from processing
 * @param rules_count
 * @param rules_id_set
 * @return number of rules being removed
 */
size_t mmt_sec_remove_rules( size_t rules_count, const uint32_t* rules_id_set );


/**
 * Add a set of rules to process
 * @param rules_mask : a string indicating special rules being attributed to special threads
 *    		e.g., "(1:10-13)(2:50)(4:1007-1010)"
 * @note: the thread_id must start from 1
 * @return number of rules being added
 */
size_t mmt_sec_add_rules( const char *rules_mask );

#endif /* SRC_LIB_MMT_SECURITY_H_ */
