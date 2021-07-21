/*
 * mmt_smp_security.h
 *
 *  Created on: Nov 17, 2016
 *  Created by: Huu Nghia NGUYEN <huunghia.nguyen@montimage.com>
 *
 *  Multi-threads version of mmt_security
 */

#ifndef SRC_LIB_MMT_SMP_SECURITY_H_
#define SRC_LIB_MMT_SMP_SECURITY_H_

#include "mmt_single_security.h"

typedef struct mmt_smp_sec_handler_struct mmt_smp_sec_handler_t;

/**
 * Register some rules to validate
 * - Input
 * 	+ threads_count: number of threads
 * 	+ core_mask    : a string indicating logical cores to be used,
 * 						  e.g., "1-8,11-12,19" => we use cores 1,2,..,8,11,12,19
 *    + rule_mask    : a string indicating special rules being attributed to special threads
 *    						e.g., "(1:10-13)(2:50)(4:1007-1010)"
 *    						The other rules will be attributed equally to the rest of threads.
 * 	+ callback     : a function to be called when a rules is validated
 * 	+ user_data    : data will be passed to the #callback
 * - Return:
 * 	+ a handler
 * - Note:
 * 	The function callback can be called from different threads. (Thus if it accesses
 * 	to a global variable or a static one, the access to these variables must be synchronous)
 */
mmt_smp_sec_handler_t *mmt_smp_sec_register(
		uint8_t threads_count,
		const uint32_t *core_mask,
		const char *rule_mask,
		bool verbose,
		mmt_sec_callback callback,
		void *user_data);

/**
 * Ignore the remain of a flow when an alert was detected on it.
 * This function must not be called after #mmt_smp_sec_process
 * @param handler
 * @param ignore
 */
void mmt_smp_sec_set_ignore_remain_flow( mmt_smp_sec_handler_t *handler, bool ignore, uint64_t buffer_size );

bool mmt_smp_is_ignore_remain_flow( mmt_smp_sec_handler_t *handler, uint64_t flow_id );

/**
 * Unregister, free resources
 */
size_t mmt_smp_sec_unregister( mmt_smp_sec_handler_t *handler, bool stop_immediatly );

/**
 * Give message to validate
 */
void mmt_smp_sec_process( mmt_smp_sec_handler_t *handler, message_t *message );


#ifdef MODULE_ADD_OR_RM_RULES_RUNTIME

void mmt_smp_sec_add_rules( mmt_smp_sec_handler_t *handler, const char*rules_mask );
void mmt_smp_sec_remove_rules( mmt_smp_sec_handler_t *handler );

#endif

#endif /* SRC_LIB_MMT_SMP_SECURITY_H_ */
