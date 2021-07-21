/*
 * rule_verif_engine.h
 *
 *  Created on: Oct 14, 2016
 *  Created by: Huu Nghia NGUYEN <huunghia.nguyen@montimage.com>
 *
 *  Verify a rule.
 *  This manages: create/free all fsm instances of a rule.
 */

#ifndef SRC_LIB_RULE_VERIF_ENGINE_H_
#define SRC_LIB_RULE_VERIF_ENGINE_H_

#include "../lib/mmt_lib.h"
#include "plugin_header.h"
#include "mmt_fsm.h"

typedef struct rule_engine_struct{
	const rule_info_t *rule_info;
	//this fsm is used for execution the first event
	fsm_t *fsm_bootstrap;

	//event_id - fsm_instance
	link_node_t **fsm_by_expecting_event_id;
	//instance_id - fsm_sub_instance
	link_node_t **fsm_by_instance_id;

	mmt_array_t *valid_execution_trace;

	//nubmer of events
	uint32_t events_count;

	uint32_t max_instances_size;
	//number of active instances
	uint32_t instances_count;
	//availabe id to allocate to a new fsm
	uint32_t avail_fsm_id;

	//depending on type of rule (verifying on a single packet or on multi-packets)
	//the processing will be different
	enum verdict_type (*processing_packets)( struct rule_engine_struct *, message_t *);

	/**
	 * An array of n elements having type of uint64_t.
	 * Each element represents required data for one event of the rule
	 */
	uint64_t *events_hash;

	//timestamp and counter of the last message being verified timeout
	uint64_t last_msg_timestamp_timeout;
	uint64_t last_msg_counter_timeout;
} rule_engine_t __aligned;

//max number of events in a rule
//max number of instances of a rule at any moment
rule_engine_t* rule_engine_init( const rule_info_t *rule_info, size_t max_instances_count );

enum verdict_type rule_engine_process( rule_engine_t *engine, message_t *message );

void rule_engine_free( rule_engine_t *engine );

const mmt_array_t* rule_engine_get_valide_trace( const rule_engine_t *_engine );

#endif /* SRC_LIB_RULE_VERIF_ENGINE_H_ */
