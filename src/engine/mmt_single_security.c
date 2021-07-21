/*
 * mmt_security.c
 *
 *  Created on: Oct 10, 2016
 *  Created by: Huu Nghia NGUYEN <huunghia.nguyen@montimage.com>
 */

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <math.h>

#include "mmt_fsm.h"
#include "plugins_engine.h"

#include "expression.h"
#include "rule.h"
#include "plugin_header.h"

#include "mmt_single_security.h"
#include "mmt_security.h"


/**
 * Public API
 */
mmt_single_sec_handler_t *mmt_single_sec_register( const rule_info_t *const*rules_array, size_t rules_count, bool verbose,
		mmt_sec_callback callback, void *user_data){
	int i, j, index;
	const rule_engine_t *engine;
	const rule_info_t *rule;
	const proto_attribute_t *p;
	uint32_t max_instance_count = conf_get_number_value( CONF_ATT__ENGINE__MAX_INSTANCES );
	__check_null( rules_array, NULL );

	mmt_single_sec_handler_t *handler = mmt_mem_alloc( sizeof( mmt_single_sec_handler_t ));
	handler->rules_count = rules_count;
	handler->rules_array = mmt_mem_alloc( sizeof (void * ) * rules_count );;
	handler->callback    = callback;
	handler->user_data_for_callback = user_data;
	handler->alerts_count = mmt_mem_alloc( sizeof (size_t ) * rules_count );
	handler->verbose      = verbose;
	handler->hash         = 0;
	handler->rules_hash   = mmt_mem_alloc( sizeof( uint64_t ) * rules_count );
	handler->flow_ids_to_ignore = NULL;
	//one fsm for one rule
	handler->engines = mmt_mem_alloc( sizeof( void *) * rules_count );
	for( i=0; i<rules_count; i++ ){
		rule = rules_array[i];

		handler->rules_array[i]  = rule;
		handler->engines[i]      = rule_engine_init( rule, max_instance_count );
		handler->alerts_count[i] = 0;

		//hash of a rule is combination of hashes of its events
		handler->rules_hash[i]   = 0;
		engine = handler->engines[i];
		for( j=0; j<engine->events_count; j++ )
			handler->rules_hash[i] |= engine->events_hash[ j ];

		//a combination of #rules_hash
		handler->hash |= handler->rules_hash[i];
	}

	handler->messages_count = 0;

#ifdef MODULE_ADD_OR_RM_RULES_RUNTIME
	pthread_spin_init( &handler->spin_lock_to_add_or_rm_rules, PTHREAD_PROCESS_PRIVATE );
#endif

	pthread_spin_init( &handler->spin_lock_to_ignore_flow, PTHREAD_PROCESS_PRIVATE );
	return handler;
}

void mmt_single_sec_set_ignore_remain_flow( mmt_single_sec_handler_t *handler, bool ignore, uint64_t buffer_size ){
	if( !ignore || handler->flow_ids_to_ignore != NULL ){
		mmt_set_ex_free( handler->flow_ids_to_ignore );
		handler->flow_ids_to_ignore = NULL;
	}

	if( ignore )
		handler->flow_ids_to_ignore = mmt_set_ex_create( buffer_size );
}

bool mmt_single_is_ignore_remain_flow( mmt_single_sec_handler_t *handler, uint64_t flow_id ){
	bool has_alerts = false;

	//the spin_lock allows this function can be called from different threads
	if( handler->flow_ids_to_ignore != NULL
			&& pthread_spin_lock( &handler->spin_lock_to_ignore_flow ) == 0 ){

		has_alerts = mmt_set_ex_check( handler->flow_ids_to_ignore, flow_id );

		pthread_spin_unlock( &handler->spin_lock_to_ignore_flow );
	}
	return has_alerts;
}

/**
 * Public API
 */
size_t mmt_single_sec_unregister( mmt_single_sec_handler_t *handler ){
	size_t i, alerts_count = 0;
	__check_null( handler, 0);

	for( i=0; i<handler->rules_count; i++ ){
		if( handler->alerts_count[ i ] == 0 )
			continue;

		if( handler->verbose ) //&& handler->rules_count > 1 )
			printf(" - rule %"PRIu32" generated %zu verdicts\n", handler->rules_array[i]->id, handler->alerts_count[ i ] );

		alerts_count += handler->alerts_count[ i ];
	}

	if( handler->flow_ids_to_ignore != NULL )
		mmt_set_ex_free( handler->flow_ids_to_ignore );

	//free data elements of handler
	for( i=0; i<handler->rules_count; i++ )
		rule_engine_free( handler->engines[i] );

	mmt_mem_free( handler->rules_array );
	mmt_mem_free( handler->engines );
	mmt_mem_free( handler->rules_hash );
	mmt_mem_free( handler->alerts_count );
	mmt_mem_free( handler );


	return alerts_count;
}

size_t mmt_single_sec_get_processed_messages( const mmt_single_sec_handler_t *handler ){
	return handler->messages_count;
}
/**
 * Public API (used by mmt_sec_smp)
 */
void mmt_single_sec_process( mmt_single_sec_handler_t *handler, message_t *msg ){
#ifdef DEBUG_MODE
	ASSERT( handler != NULL, "msg cannot be null");
	ASSERT( msg != NULL, "msg cannot be null");
#endif

	size_t i, j;
	int verdict;
	const mmt_array_t *execution_trace;

	BEGIN_LOCK_IF_ADD_OR_RM_RULES_RUNTIME( &handler->spin_lock_to_add_or_rm_rules )

	//the message does not concern to any rules handled by this #handler
	//as it does not contain any proto.att required by the handler
	if( unlikely((msg->hash & handler->hash) == 0 ))
		goto _finish;

	//if the message can be ignored as it is in a flow that has been detected an alert
	if( mmt_single_is_ignore_remain_flow( handler, msg->flow_id ))
		goto _finish;

	handler->messages_count ++;

	//for each rule
	for( i=0; i<handler->rules_count; i++){

		//msg does not contain any proto.att for i-th rule
		if( (msg->hash & handler->rules_hash[i]) == 0 )
			continue;

//		DEBUG("%"PRIu64" verify rule %d", msg->counter, handler->rules_array[i]->id );

		verdict = rule_engine_process( handler->engines[i], msg );

		//found a validated/invalid trace
		if( unlikely( verdict != VERDICT_UNKNOWN )){
			handler->alerts_count[i] ++;

			//get execution trace
			execution_trace = rule_engine_get_valide_trace( handler->engines[i] );

			//callback fucntion of rule
			if( handler->rules_array[i]->if_satisfied != NULL )
				handler->rules_array[i]->if_satisfied(
						handler->rules_array[i],
						verdict,
						msg->timestamp,
						msg->counter,
						execution_trace );

			//call user-callback function
			if( handler->callback != NULL ){
				handler->callback(
					handler->rules_array[i],
					verdict,
					msg->timestamp,
					msg->counter,
					execution_trace,
					handler->user_data_for_callback );
			}

			//if we need to ignore the messages in a flow that has been detected an alert
			if( handler->flow_ids_to_ignore != NULL
					&& pthread_spin_lock( &handler->spin_lock_to_ignore_flow ) == 0 ){

				uint64_t last_flow_id = -1;
				for( j=0; j<execution_trace->elements_count; j++ ){
					message_t *m = (message_t *) execution_trace->data[j];
					if( m == NULL ||
							//usually the messages in the trace are in the same flow
							m->flow_id == last_flow_id
						)
						continue;

					last_flow_id = m->flow_id;
					mmt_set_ex_add( handler->flow_ids_to_ignore, m->flow_id );
				}

				pthread_spin_unlock( &handler->spin_lock_to_ignore_flow );
			}
		}
	}

	_finish:

	UNLOCK_IF_ADD_OR_RM_RULES_RUNTIME( &handler->spin_lock_to_add_or_rm_rules )
	END_LOCK_IF_ADD_OR_RM_RULES_RUNTIME
	free_message_t( msg );
}


#ifdef MODULE_ADD_OR_RM_RULES_RUNTIME
static inline void _swap_rule( mmt_single_sec_handler_t *handler, int i, int j ){
	const rule_info_t *rule;
	rule_engine_t *engine;

	size_t tmp;
	uint64_t hash;

	if( i==j )
		return;

	//swap rule info
	rule = handler->rules_array[ i ];
	handler->rules_array[ i ] = handler->rules_array[ j ];
	handler->rules_array[ j ] = rule;

	//swap rule engine
	engine = handler->engines[ i ];
	handler->engines[ i ] = handler->engines[ j ];
	handler->engines[ j ] = engine;

	//swap alert count
	tmp = handler->alerts_count[ i ];
	handler->alerts_count[ i ] = handler->alerts_count[ j ];
	handler->alerts_count[ j ] = tmp;

	//swap rule hash
	hash = handler->rules_hash[ i ];
	handler->rules_hash[ i ] = handler->rules_hash[ j ];
	handler->rules_hash[ j ] = hash;
}

/**
 *
 * @param handler
 * @param i
 */
static inline void _free_rule( mmt_single_sec_handler_t *handler, size_t i ){
	if( handler->verbose )
		printf(" - rule %"PRIu32" generated %zu verdicts\n", handler->rules_array[i]->id, handler->alerts_count[ i ] );
	rule_engine_free( handler->engines[i] );
	handler->engines[i] = NULL;
}


//this function is implemented inside mmt_security.c to get the set of current rules to be verified
size_t _mmt_sec_get_rules_info_without_lock( rule_info_t const*const**rules_array );

//PUBLIC_API
size_t mmt_single_sec_remove_rules( mmt_single_sec_handler_t *handler){
	size_t i, j;
	size_t removed_rules_count = 0;
	const rule_info_t *rule;
	size_t rules_count;
	rule_info_t const*const* rules;

	rules_count = _mmt_sec_get_rules_info_without_lock( &rules );

	BEGIN_LOCK_IF_ADD_OR_RM_RULES_RUNTIME( & handler->spin_lock_to_add_or_rm_rules )

	//A rule is to removed if
	// (1) it exists in #handler->rules_array
	// and (2) it does not exist in #rules
	i = 0;
	while( i<handler->rules_count ){
		rule = handler->rules_array[i];

		//check if this #rule is existing in #rules
		for( j=0; j<rules_count; j++ )
			if( rule->id == rules[j]->id )
				break;

		//FOUND a rule => retain #rule
		if( j < rules_count ){
			i ++; //jump to the next rule
			continue;
		}

		//move the rule to be remove to the end
		_swap_rule( handler, i, handler->rules_count - 1 );

		//remove the rule that is now at the end
		_free_rule( handler, handler->rules_count - 1 );

		//reduce number of rules
		handler->rules_count --;

		removed_rules_count ++;
	}

	//update global hash if need
	if( removed_rules_count > 0 ){
		handler->hash = 0;
		for( i=0; i<handler->rules_count; i++ )
			handler->hash |= handler->rules_hash[i];
	}

	UNLOCK_IF_ADD_OR_RM_RULES_RUNTIME( &handler->spin_lock_to_add_or_rm_rules )
	END_LOCK_IF_ADD_OR_RM_RULES_RUNTIME

	return removed_rules_count;
}

//PUBLIC_API
size_t mmt_single_sec_add_rules( mmt_single_sec_handler_t *handler, size_t new_rules_id_arr_size,
		const uint32_t *new_rules_id_arr ){

	//check parameter
	__check_null( handler, 0 );
	__check_zero( new_rules_id_arr_size, 0 );

	size_t i, j, k;

	const rule_info_t **old_rules_array, *rule;
	rule_engine_t **old_engines;
	size_t *old_alerts_count;
	uint64_t *old_rules_hash;

	//global rules set
	rule_info_t const*const*global_rules_set;
	size_t global_total_rules_count = _mmt_sec_get_rules_info_without_lock( &global_rules_set );

	//set of rules to be added: maximally all of rules will be added
	const rule_info_t **rules_set_to_be_add;
	size_t add_rules_count = 0;
	rules_set_to_be_add =  mmt_mem_alloc( new_rules_id_arr_size  * sizeof( void * ));

	//filter out the id in #new_rules_id_arr that is not id of any rule
	//a rule is added if
	// (1) it exist in #global_rules_set
	// (2) its id exists in #new_rules_id_arr
	// (3) it is not in handler->rules_array

	for( i=0; i<global_total_rules_count; i++ ){
		//(1)
		rule = global_rules_set[ i ];
		//(2)
		if( index_of( rule->id, new_rules_id_arr, new_rules_id_arr_size) == new_rules_id_arr_size )
			continue;
		//(3)
		for( j=0; j<handler->rules_count; j++ )
			if( rule->id == handler->rules_array[j]->id )
				break;
		//==> exist in handler->rules_array
		if( j < handler->rules_count )
			continue;

		rules_set_to_be_add[ add_rules_count ] = rule;
		add_rules_count ++;
	}

	//nothing to be added?
	if( add_rules_count == 0 ){
		mmt_mem_free( rules_set_to_be_add );
		return 0;
	}

	//old_rules_array
	old_rules_array  = handler->rules_array;
	old_engines      = handler->engines;
	old_alerts_count = handler->alerts_count;
	old_rules_hash   = handler->rules_hash;

	BEGIN_LOCK_IF_ADD_OR_RM_RULES_RUNTIME( &handler->spin_lock_to_add_or_rm_rules )
	//extends the current arrays by create a new one
	handler->rules_array  = mmt_mem_alloc( sizeof( void *)    * ( handler->rules_count + add_rules_count ));
	handler->engines      = mmt_mem_alloc( sizeof( void *)    * ( handler->rules_count + add_rules_count ));
	handler->alerts_count = mmt_mem_alloc( sizeof( size_t )   * ( handler->rules_count + add_rules_count ));
	handler->rules_hash   = mmt_mem_alloc( sizeof( uint64_t ) * ( handler->rules_count + add_rules_count ));

	//retake the old values
	for( i=0; i<handler->rules_count; i++ ){
		handler->rules_array[ i ]  = old_rules_array[ i ];
		handler->engines[ i ]      = old_engines[ i ];
		handler->alerts_count[ i ] = old_alerts_count[ i ];
		handler->rules_hash[ i ]   = old_rules_hash[ i ];
	}

	//free the old memories
	mmt_mem_free( old_rules_array  );
	mmt_mem_free( old_engines      );
	mmt_mem_free( old_alerts_count );
	mmt_mem_free( old_rules_hash   );

	size_t max_instances_count = conf_get_number_value( CONF_ATT__ENGINE__MAX_INSTANCES );
	//add the new rules
	j = 0;
	for( i=handler->rules_count; i< (handler->rules_count + add_rules_count); i++ ){

		//init variables for this rule
		handler->rules_array[ i ]  = rules_set_to_be_add[ j ];
		handler->engines[ i ]      = rule_engine_init( handler->rules_array[ i ], max_instances_count );
		handler->alerts_count[ i ] = 0;
		handler->rules_hash[ i ]   = 0;
		//hash number for this rule
		for( k=0; k<handler->engines[i]->events_count; k++ )
			handler->rules_hash[i] |= handler->engines[i]->events_hash[ k ];

		//update global hash
		handler->hash |= handler->rules_hash[i];

		j++;
	}

	//new rules size
	handler->rules_count += add_rules_count;

	UNLOCK_IF_ADD_OR_RM_RULES_RUNTIME( &handler->spin_lock_to_add_or_rm_rules )
	END_LOCK_IF_ADD_OR_RM_RULES_RUNTIME

	//free memory
	mmt_mem_free( rules_set_to_be_add );
	return add_rules_count;
}
#endif
