/*
 * rule_verif_engine.c
 *
 *  Created on: Oct 14, 2016
 *  Created by: Huu Nghia NGUYEN <huunghia.nguyen@montimage.com>
 */
#include "rule_verif_engine.h"
#include "../lib/prefetch.h"
#include "mmt_fsm.h"
#include "mmt_security.h"
/**
 * In the next event,
 * the #fsm will fire the #index-th transition of its current state
 */
typedef struct _fsm_tran_index_struct{
	uint64_t counter;     //index of the last verified message
	size_t index;         //index of the transition of the current state of #fsm to be verified
	fsm_t *fsm;           //fsm to be verified
} __aligned _fsm_tran_index_t ;

static inline _fsm_tran_index_t* _create_fsm_tran_index_t( rule_engine_t *_engine, size_t index, fsm_t *fsm, uint64_t counter){

	_fsm_tran_index_t *ret = mmt_mem_alloc( sizeof( _fsm_tran_index_t));
	ret->index   = index;
	ret->fsm     = fsm;
	ret->counter = counter;
	return ret;
}

/**
 * Index the transitions of #fsm that can be fired in the next event.
 */
static inline
void _set_expecting_events_id( rule_engine_t *_engine, fsm_t *fsm, bool is_add_timeout, uint64_t counter ){
	size_t i;
	uint16_t event_id;
	const fsm_transition_t *tran;
	const fsm_state_t *state = fsm_get_current_state( fsm );
	_fsm_tran_index_t *fsm_ind;

	//from a state: 2 outgoing transitions have 2 different events
#ifdef DEBUG_MODE
	ASSERT( _engine->events_count >= state->transitions_count,
			"Error: Number of outgoing transition must not be greater than number of events (%zu <= %d)",
			state->transitions_count, _engine->events_count );
#endif

	//for each outgoing transition, we add it to the list of expecting events
	for( i=0; i<state->transitions_count; i++ ){
		//i == 0: timeout => not always

		tran     = &( state->transitions[ i ] );
		event_id = tran->event_type;

		if( event_id == FSM_EVENT_TYPE_TIMEOUT && !is_add_timeout )
			continue;

//d = count_nodes_from_link_list( _engine->fsm_by_expecting_event_id[ event_id ]);
//ASSERT( d<= 300, "Stop here, total ins: %zu", _engine->total_instances_count );

//DEBUG( "%d: event_id %d, event_type: %d, event_index: %zu, max: %zu, number of ins: %zu",
//		_engine->rule_info->id,
//		event_id, tran->event_type, i, _engine->max_events_count,
//		d);

		//if event having #event_id occurs,
		// then #fsm will fire the i-th transition from its current state
		fsm_ind = _create_fsm_tran_index_t( _engine, i, fsm, counter );

		_engine->fsm_by_expecting_event_id[ event_id ] =
				insert_node_to_link_list( _engine->fsm_by_expecting_event_id[ event_id], fsm_ind );
	}
}

static inline enum verdict_type _get_verdict( int rule_type, enum fsm_handle_event_value result ){
	switch ( rule_type ) {
	case RULE_TYPE_TEST:
		switch( result ){
		case FSM_ERROR_STATE_REACHED:
			return VERDICT_NOT_RESPECTED;
		case FSM_FINAL_STATE_REACHED:
			return VERDICT_RESPECTED;
		default:
			return VERDICT_UNKNOWN;
		}
		break;

	case RULE_TYPE_SECURITY:
		switch( result ){
		case FSM_ERROR_STATE_REACHED:
			return VERDICT_NOT_RESPECTED;
		case FSM_FINAL_STATE_REACHED:
//			return VERDICT_RESPECTED;
			return VERDICT_UNKNOWN;
		default:
			return VERDICT_UNKNOWN;
		}
		break;
	case RULE_TYPE_ATTACK:
	case RULE_TYPE_EVASION:
		switch( result ){
		case FSM_ERROR_STATE_REACHED:
			return VERDICT_UNKNOWN; //VERDICT_NOT_DETECTED;
		case FSM_FINAL_STATE_REACHED:
			return VERDICT_DETECTED;
		default:
			return VERDICT_UNKNOWN;
		}
		break;

	case RULE_TYPE_FORWARD:
		switch( result ){
		case FSM_ERROR_STATE_REACHED:
			return VERDICT_UNKNOWN; //VERDICT_NOT_DETECTED;
		case FSM_FINAL_STATE_REACHED:
			return VERDICT_DETECTED;
		default:
			return VERDICT_UNKNOWN;
		}
		break;

	default:
		ABORT("Error 22: Property type should be a security rule or an attack.\n");
	}//end of switch
	return VERDICT_UNKNOWN;
}




static inline
void _store_valid_execution_trace( rule_engine_t *_engine, fsm_t *fsm ){
	size_t i;
	const mmt_array_t *array = fsm_get_execution_trace( fsm );

#ifdef DEBUG_MODE
	ASSERT( array->elements_count == _engine->events_count,
			"Error when processing rule %d: trace_size != event_count( %zu != %"PRIu32")",
			_engine->rule_info->id, array->elements_count, _engine->events_count );
#endif

	for( i=0; i<_engine->events_count; i++ ){
		//free old message
		free_message_t( _engine->valid_execution_trace->data[i] );

		//increase number of references to each message
		_engine->valid_execution_trace->data[i] = mmt_mem_atomic_retain( array->data[i] );
	}
}

/**
 * Public API
 */
const mmt_array_t* rule_engine_get_valide_trace( const rule_engine_t *engine ){
#ifdef DEBUG_MODE
	ASSERT( engine != NULL, "engine cannot be null" );
#endif

	rule_engine_t *_engine = ( rule_engine_t *)engine;
	return _engine->valid_execution_trace;
}


/**
 * Remove all instances (fsm) having same id with #fsm
 */
static inline
void _reset_engine_for_fsm( rule_engine_t *_engine, fsm_t *fsm ){
	size_t i;
	link_node_t *node, *ptr;
	_fsm_tran_index_t *fsm_ind;
	uint16_t fsm_id = fsm_get_id( fsm );
	//remove all fsm having #fsm_id from the list #fsm_by_expecting_event_id
	for( i=0; i<_engine->events_count; i++ ){
		node = _engine->fsm_by_expecting_event_id[ i ];
		while( node != NULL ){
			ptr = node->next;
//			prefetch2( ptr );
			fsm_ind = (_fsm_tran_index_t *) node->data;

			//found a node containing fsm having id = #fsm_id
			if( fsm_get_id( fsm_ind->fsm ) == fsm_id ){

				//head?
				if( node == _engine->fsm_by_expecting_event_id[ i ] ){
					if( ptr != NULL ) ptr->prev = NULL;
					_engine->fsm_by_expecting_event_id[ i ] = ptr;
				}else{
					node->prev->next = ptr;
					if( ptr != NULL )
						ptr->prev = node->prev;
				}
				mmt_mem_force_free( fsm_ind );//node->data

				mmt_mem_force_free( node );
			}
			node = ptr;
		}
	}
	//remove all fsm having id == #fsm_id and free them
	//TODO: refine
//	_engine->total_instances_count -= count_nodes_from_link_list( _engine->fsm_by_instance_id[ fsm_id ] );
	//ASSERT( _engine->total_instances_count >= 0, "Cannot be negative. %s:%d", __FILE__, __LINE__ );
	free_link_list_and_data( _engine->fsm_by_instance_id[ fsm_id ], (void *)fsm_free );

	//put it to be available for the other
	_engine->fsm_by_instance_id[ fsm_id ] = NULL;

	//fsm_id become avaiable for other
	_engine->avail_fsm_id = fsm_id;
}
/**
 * Remove only one instance from
 * - (1) list of expecting events, and from
 * - (2) list of instances
 */
static inline
void _reset_engine_for_instance( rule_engine_t *_engine, fsm_t *fsm ){
	size_t i;
	link_node_t *node, *ptr;
	_fsm_tran_index_t *fsm_ind;
	uint16_t fsm_id = fsm_get_id( fsm );

	//(1) - remove fsm from the list #fsm_by_expecting_event_id

	//for each entry (that is a list) of expecting event i
	for( i=0; i<_engine->events_count; i++ ){
		node = _engine->fsm_by_expecting_event_id[ i ];
		while( node != NULL ){
			ptr     = node->next;
//			prefetch2( ptr );
			fsm_ind = (_fsm_tran_index_t *) node->data;

			//found a node containing #fsm
			if( fsm_ind->fsm == fsm ){

				//head?
				if( node == _engine->fsm_by_expecting_event_id[ i ] ){
					if( ptr != NULL ) ptr->prev = NULL;
					_engine->fsm_by_expecting_event_id[ i ] = ptr;
				}else{
					node->prev->next = ptr;
					if( ptr != NULL )
						ptr->prev = node->prev;
				}
				mmt_mem_force_free( fsm_ind );//node->data

				mmt_mem_force_free( node );
			}
			node = ptr;
		}
	}

	//(2) - remove fsm from the list of instances
	//TODO: refine
//	_engine->total_instances_count -= count_nodes_from_link_list( _engine->fsm_by_instance_id[ fsm_id ] );
	_engine->fsm_by_instance_id[ fsm_id ] = remove_node_from_link_list(  _engine->fsm_by_instance_id[ fsm_id ], fsm );
	if( _engine->fsm_by_instance_id[ fsm_id ] == NULL )
		_engine->avail_fsm_id = fsm_id;

	fsm_free( fsm );
}

static inline
uint16_t _find_an_available_id( rule_engine_t *_engine ){
	size_t i = _engine->avail_fsm_id;
	for( ; i<_engine->max_instances_size; i++ )
		if( _engine->fsm_by_instance_id[ i ] == NULL ){
			//remember this id being available
			_engine->avail_fsm_id = i;
			return i;
		}

	//not enough
	ABORT( "Not enough memory slots for fsm instances when verifying rule %d. Need to increase %s",
			_engine->rule_info->id, conf_get_name_from_id( CONF_ATT__ENGINE__MAX_INSTANCES ));
	return 0;
}

static inline
enum verdict_type _process_a_node( link_node_t *node, uint16_t event_id, message_t *message, rule_engine_t *_engine ){
	_fsm_tran_index_t *fsm_ind = node->data;
	fsm_t *fsm = fsm_ind->fsm;
	//fire a specific transition of the current state of #node->fsm
	//the transition has index = #node->tran_index
	enum fsm_handle_event_value val;
	fsm_t *new_fsm = NULL;
	uint16_t new_fsm_id;
	enum verdict_type verdict;

	//DEBUG( "  transition to verify: %zu", fsm_ind->index );
	val = fsm_handle_event( fsm, fsm_ind->index, message, &new_fsm );

//	DEBUG( "Verify transition: %zu of fsm %p", fsm_ind->index, fsm );

	//if the execution of the transition having index = fsm_ind->index creates a new fsm
	if( new_fsm != NULL ){
		//the new fsm being created is an instance of #fsm_bootstrap
		//=> put the new one to a new class
		if( fsm == _engine->fsm_bootstrap ){
			new_fsm_id = _find_an_available_id( _engine );
			fsm_set_id( new_fsm, new_fsm_id );
		}else
			new_fsm_id = fsm_get_id( new_fsm );

		//add the new_fsm to the list of fsm(s) having the same id
		_engine->fsm_by_instance_id[ new_fsm_id  ] = insert_node_to_link_list( _engine->fsm_by_instance_id[ new_fsm_id ], new_fsm );

		//TODO: refine this
//		if( _engine->total_instances_count >= 400 ){
//			rule_engine_free( _engine );
//			ABORT( "Too big %s:%d", __FILE__, __LINE__ );
//			DEBUG("Number of instances: %zu, fsm_id: %d", _engine->total_instances_count, new_fsm_id );
//		}

		fsm = new_fsm;
//		fsm_set_user_data( fsm, _engine );
	}
	else{
		if( val == FSM_STATE_CHANGED ){
			//DEBUG( "FSM_STATE_CHANGED" );

			//remove from old list
			//=> the #fsm_ind->fsm does not wait for the #event_id any more
			_engine->fsm_by_expecting_event_id[ event_id ] = remove_link_node_from_its_link_list( node, _engine->fsm_by_expecting_event_id[ event_id ] );

			//free the node
			mmt_mem_force_free( node );
			//free node->data
			mmt_mem_force_free( fsm_ind );
		}
	}

	switch( val ){
	case FSM_STATE_CHANGED:
		//DEBUG( "FSM_STATE_CHANGED" );
		//add to new list(s) that waiting for a new event
		//add also TIMEOUT event for the first time
		_set_expecting_events_id( _engine, fsm, new_fsm != NULL,  message->counter );
		break;

	//a final state that is either valid state or invalid one
	//depending on rule type (ATTACK, SECURITY, EVASION, ... ) a different verdict will be given
	case FSM_FINAL_STATE_REACHED:
	case FSM_ERROR_STATE_REACHED:
		verdict = _get_verdict( _engine->rule_info->type_id, val);

		if( verdict == VERDICT_UNKNOWN ){
			//remove the current fsm from validating list
			_reset_engine_for_instance( _engine, fsm );
		}else{
			_store_valid_execution_trace( _engine, fsm );
			//remove all fsm having the same id => stop validating this fsm and its instances (the other fsm having the same id)
			_reset_engine_for_fsm( _engine, fsm );
		}
		return verdict;

	case FSM_INCONCLUSIVE_STATE_REACHED:
		_reset_engine_for_instance( _engine, fsm );
		break;

#ifdef DEBUG_MODE
	case FSM_ERR_ARG:
		ABORT( "FSM_ERR_ARG" );
		break;
#endif

	default:
		break;
	}

	return VERDICT_UNKNOWN;
}

/**
 * Public API
 */
enum verdict_type rule_engine_process( rule_engine_t *engine, message_t *message ){
#ifdef DEBUG_MODE
	ASSERT( engine != NULL,  "engine cannot be null" );
	ASSERT( message != NULL, "message cannot be null" );
#endif
	return engine->processing_packets( engine, message );
}


enum verdict_type _process_single_packet( rule_engine_t *engine, message_t *message ){
	enum verdict_type verdict;
	enum fsm_handle_event_value val;
	const mmt_array_t *array;
	size_t i;

	//check if message contains enough data to verify 2 consecutive events of the rule
	if( (message->hash & engine->events_hash[ 1 ]) != engine->events_hash[ 1 ] ||
		 (message->hash & engine->events_hash[ 2 ]) != engine->events_hash[ 2 ] ){
//		DEBUG("Filtered out %"PRIu64" verify rule %d", message->counter, engine->rule_info->id );
		return VERDICT_UNKNOWN;
	}

//	DEBUG("%"PRIu64" verify rule %d", message->counter, engine->rule_info->id );

	verdict = VERDICT_UNKNOWN;
	val     = fsm_handle_single_packet( engine->fsm_bootstrap, message );
	switch( val ){
	case FSM_NO_STATE_CHANGE:
		return VERDICT_UNKNOWN;

	//a final state that is either valid state or invalid one
	//depending on rule type (ATTACK, SECURITY, EVASION, ... ) a different verdict will be given
	case FSM_INCONCLUSIVE_STATE_REACHED:
	case FSM_FINAL_STATE_REACHED:
	case FSM_ERROR_STATE_REACHED:
		verdict = _get_verdict( engine->rule_info->type_id, val );

		if( verdict != VERDICT_UNKNOWN )
			_store_valid_execution_trace( engine, engine->fsm_bootstrap );

		//reset #fsm_bootstrap so it is clean for the next tests
		fsm_reset( engine->fsm_bootstrap );

		break;

	case FSM_ERR_ARG:
		ABORT( "FSM_ERR_ARG" );
		break;

	default:
		break;
	}

	return verdict;
}


enum verdict_type _process_multi_packets( rule_engine_t *engine, message_t *message ){
	uint8_t event_id;
	link_node_t *node, *node_ptr;
	_fsm_tran_index_t *fsm_ind;
	enum verdict_type ret;

	//DEBUG( "Verify message counter: %"PRIu64", ts: %"PRIu64, message->counter, message->timestamp );
	//DEBUG( "===Verify Rule %d=== %zu", _engine->rule_info->id, _engine->max_events_count );

	//get from hash table the list of events to be verified
	for( event_id=0; event_id<engine->events_count; event_id++ ){

		//message does not contain enough data required by this event
		if( (message->hash & engine->events_hash[ event_id ] ) != engine->events_hash[ event_id ] )
			continue;

//		DEBUG( "Event_id : %d", event_id );

		//verify fsm instances that are waiting for event_id
		node = engine->fsm_by_expecting_event_id[ event_id ];

		//for each instance
		while( node != NULL ){

			fsm_ind  = (_fsm_tran_index_t *)node->data;
			node_ptr = node;
			node     = node->next;

			//check whether the #fsm_ind was verified above
			if( unlikely( fsm_ind->counter == message->counter ))
				continue;

			//the message will be processed by this fsm_ind
			fsm_ind->counter = message->counter;

			//put this after node = node->next
			// because #node can be freed( or inserted a new node) in the function #_fire_transition
			ret = _process_a_node( node_ptr, event_id, message, engine );

			//get only one verdict per packet
			if( ret != VERDICT_UNKNOWN ){
				return ret;
			}
		}
	}

	//verify timeout event for each 10 messages or an interval of 100000 usec (0.1s)

	if( engine->last_msg_counter_timeout < message->counter + 10
			&& engine->last_msg_timestamp_timeout < message->timestamp + 100000 )
		return VERDICT_UNKNOWN;

	engine->last_msg_counter_timeout   = message->counter;
	engine->last_msg_timestamp_timeout = message->timestamp;


	//check timeout
	//verify instances that are waiting for timeout
	node = engine->fsm_by_expecting_event_id[ FSM_EVENT_TYPE_TIMEOUT ];
	//DEBUG( "Zero-nodes count: %zu", count_nodes_from_link_list(node));

	while( node != NULL ){

		fsm_ind  = (_fsm_tran_index_t *)node->data;
		node_ptr = node;
		node     = node->next;

		//check whether the #fsm_ind was verified above
		if( fsm_ind->counter == message->counter )
			continue;

		//the message will be processed by the machine fsm_ind->fsm
		fsm_ind->counter = message->counter;

		//put this after node = node->next
		// because #node can be freed( or inserted a new node) in the function #_fire_transition
		ret = _process_a_node( node_ptr, FSM_EVENT_TYPE_TIMEOUT, message, engine );

		//get only one verdict per packet
		if( ret != VERDICT_UNKNOWN ){
			return ret;
		}
	}

	return VERDICT_UNKNOWN;
}

//this is defined in mmt_security.c
#ifdef MODULE_ADD_OR_RM_RULES_RUNTIME
uint16_t _mmt_sec_hash_proto_attribute_without_lock( uint32_t proto_id, uint32_t att_id );
#endif
/**
 * Calculate hash of each event
 * This is called only one time at initiation of #_engine
 * @param _engine
 */
static inline void _calculate_hash_number( rule_engine_t *_engine ){
	int i, j, k, index;
	const rule_info_t *rule = _engine->rule_info;
	const mmt_array_t *ev;
	const proto_attribute_t *p;
	bool is_in_excluded_list;
	for( i=0; i<_engine->events_count; i++ ){
		_engine->events_hash[i] = 0;
		ev = &( rule->proto_atts_events[i] );

		//for each proto.att
		for( j=0; j<ev->elements_count; j++ ){
			p = ev->data[ j ];

			is_in_excluded_list = false;
			//check whether p in the excluded list
			for( k=0; k<rule->excluded_filter[i].elements_count; k++ )
				if( p == rule->excluded_filter[i].data[ k ] ){
					is_in_excluded_list = true;
					DEBUG("Exclude present checking of %s.%s from event %d of rule %d", p->proto, p->att, i, rule->id );
					break;
				}

			/*
			 * When a rule need to explicitly exclude some proto_att from the filter (done by hash)
			 */
			if( !is_in_excluded_list ){
#ifdef MODULE_ADD_OR_RM_RULES_RUNTIME
				index = _mmt_sec_hash_proto_attribute_without_lock( p->proto_id, p->att_id );
#else
				index = mmt_sec_hash_proto_attribute( p->proto_id, p->att_id );
#endif

				BIT_SET( _engine->events_hash[i], index );
			}
		}

	}
}

/**
 * Public API
 */
rule_engine_t* rule_engine_init( const rule_info_t *rule_info, size_t max_instances_count ){
	size_t i;
	ASSERT( rule_info != NULL, "rule_info is NULL");
	rule_engine_t *_engine = mmt_mem_alloc( sizeof( rule_engine_t ));

	_engine->fsm_bootstrap      = rule_info->create_instance();
	fsm_set_id( _engine->fsm_bootstrap, 0 );

	_engine->rule_info     = rule_info;
	_engine->events_count  = rule_info->events_count + 1; //event_id start from 1 (0 is timeout)
	ASSERT( _engine->events_count <= 64, "Cannot hold more than 64 events in a property" );
	_engine->max_instances_size = max_instances_count;
	_engine->instances_count    = 1; //fsm_bootstrap
	_engine->avail_fsm_id       = 0;
	//linked-list of fsm instances indexed by their expected event_id
	_engine->fsm_by_expecting_event_id = mmt_mem_alloc( _engine->events_count * sizeof( void *) );
	for( i=0; i<_engine->events_count; i++ )
		_engine->fsm_by_expecting_event_id[ i ] = NULL;

	//add fsm_bootstrap to the list
	_set_expecting_events_id( _engine, _engine->fsm_bootstrap, NO, 0 );

	//linked-list of fsm instances
	_engine->fsm_by_instance_id = mmt_mem_alloc( _engine->max_instances_size * sizeof( void *) );
	for( i=0; i<_engine->max_instances_size; i++ )
		_engine->fsm_by_instance_id[ i ] = NULL;

	//add fsm_bootstrap to the first element of
	_engine->fsm_by_instance_id[ 0 ] = insert_node_to_link_list(_engine->fsm_by_instance_id[ 0 ], _engine->fsm_bootstrap );

	_engine->valid_execution_trace = mmt_array_init( _engine->events_count );

	_engine->events_hash = mmt_mem_alloc( sizeof( uint64_t) * _engine->events_count );
	_calculate_hash_number( _engine );
	fsm_set_user_data( _engine->fsm_bootstrap, _engine );

	_engine->last_msg_counter_timeout = 0;
	_engine->last_msg_timestamp_timeout    = 0;

	if( fsm_is_verifying_single_packet( _engine->fsm_bootstrap) ){
		_engine->processing_packets = _process_single_packet;
	}else{
		_engine->processing_packets = _process_multi_packets;
	}

	return _engine;
}


/**
 * Public API
 */
void rule_engine_free( rule_engine_t *_engine ){
	size_t i;

	__check_null( _engine, );

	for( i=0; i<_engine->events_count; i++ )
		free_link_list(_engine->fsm_by_expecting_event_id[ i ], YES );
	_engine->events_count = 0;

	mmt_mem_free( _engine->fsm_by_expecting_event_id );

	for( i=0; i<_engine->max_instances_size; i++ )
		free_link_list_and_data( _engine->fsm_by_instance_id[ i ], (void *)fsm_free );

	mmt_mem_free( _engine->fsm_by_instance_id );
	mmt_mem_free( _engine->events_hash );

	if( _engine->valid_execution_trace != NULL )
		mmt_array_free( _engine->valid_execution_trace, (void *)free_message_t );

	mmt_mem_free( _engine );
}
