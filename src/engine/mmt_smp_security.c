/*
 * mmt_smp_security.c
 *
 *  Created on: Nov 17, 2016
 *  Created by: Huu Nghia NGUYEN <huunghia.nguyen@montimage.com>
 */


#include <pthread.h>
#include <errno.h>
#include <unistd.h>

#include "mmt_smp_security.h"
#include "../lib/mmt_lib.h"

struct mmt_smp_sec_handler_struct{
	size_t threads_count;

	mmt_single_sec_handler_t **mmt_single_sec_handlers;
	pthread_t *threads_id;

	//one ring per thread
	lock_free_spsc_ring_t **rings;

	bool verbose;
}__aligned;

struct _thread_arg{
	uint32_t index; //index of thread
	uint32_t lcore; //lcore id on which thread will run on
	const mmt_smp_sec_handler_t *handler;
};


static inline void _mmt_smp_sec_stop( mmt_smp_sec_handler_t *handler, bool stop_immediately  ){
	size_t i;
	int ret;

	if( stop_immediately ){
		for( i=0; i<handler->threads_count; i++ )
			pthread_cancel( handler->threads_id[ i ] );
	}else{
		//insert NULL message at the end of ring
		mmt_smp_sec_process( handler, NULL );

		//waiting for all threads finish their job
		for( i=0; i<handler->threads_count; i++ ){
			ret = pthread_join( handler->threads_id[ i ], NULL );
			switch( ret ){
			case EDEADLK:
				log_write( LOG_ERR,"A deadlock was detected or thread specifies the calling thread");
				break;
			case EINVAL:
				log_write( LOG_ERR,"Thread is not a joinable thread.");
				break;
//			case EINVAL:
//				ABORT("Another thread is already waiting to join with this thread.");
//				break;
			case  ESRCH:
				log_write( LOG_ERR,"No thread with the ID thread could be found.");
				break;
			}
		}
	}
}

/**
 * Public API
 */
size_t mmt_smp_sec_unregister( mmt_smp_sec_handler_t *handler, bool stop_immediately ){
	size_t i, total_alerts = 0, alerts_count, messages_count;

	__check_null( handler, 0);

	_mmt_smp_sec_stop( handler, stop_immediately );

	//free data elements of handler
	for( i=0; i<handler->threads_count; i++ ){
		messages_count = mmt_single_sec_get_processed_messages( handler->mmt_single_sec_handlers[i] );
		alerts_count   = mmt_single_sec_unregister( handler->mmt_single_sec_handlers[i] );
		if( handler->verbose )
			log_write( LOG_INFO,"MMT-5Greplay thread %2zu processed %8zu messages and generated %8zu alerts", i+1, messages_count, alerts_count );

		total_alerts += alerts_count;
	}

	for( i=0; i<handler->threads_count; i++ )
		ring_free( handler->rings[ i ] );

	mmt_mem_free( handler->rings );
	mmt_mem_free( handler->mmt_single_sec_handlers );
	mmt_mem_free( handler->threads_id );

	mmt_mem_free( handler );
	return total_alerts;
}


static inline void *_process_one_thread( void *arg ){
	struct _thread_arg *thread_arg = (struct _thread_arg *) arg;
	mmt_single_sec_handler_t *mmt_sec = thread_arg->handler->mmt_single_sec_handlers[ thread_arg->index ];
	lock_free_spsc_ring_t *ring       = thread_arg->handler->rings[ thread_arg->index ];

	void *array[100];
	const size_t length = 100;
	size_t size, i;

	pthread_setcanceltype( PTHREAD_CANCEL_ENABLE, NULL );

	if( thread_arg->lcore != 0 && move_the_current_thread_to_a_processor( thread_arg->lcore, -14 ))
		log_write( LOG_WARNING,"Cannot set affinity of thread %d on lcore %d", gettid(), thread_arg->lcore  );

	while( 1 ){
		//get all possible messages in ring
		size = ring_pop_burst( ring, length, array );

		if( size == 0 )
			ring_wait_for_pushing( ring );
		else{
			//do not process the last msg in the for-loop
			size -= 1;
			for( i=0; i< size; i++ )
				mmt_single_sec_process( mmt_sec, array[i] );

			//only the last msg can be NULL
			if( unlikely( array[ size ] == NULL ) ){
				break;
			}else{
				mmt_single_sec_process( mmt_sec, array[size] );
			}
		}
	}

	mmt_mem_free( thread_arg );

	return NULL;
}

/**
 * Public API
 */
mmt_smp_sec_handler_t *mmt_smp_sec_register(
		uint8_t threads_count, const uint32_t *core_mask, const char* rule_mask, bool verbose,
		mmt_sec_callback callback, void *user_data){
	int i, j, k, rules_count_per_thread;

	const rule_info_t **rule_ptr, *tmp, **all_rules;
	int ret;
	struct _thread_arg *thread_arg;
	long cpus_count = get_number_of_online_processors() - 1;
	const size_t ring_len = 10000 - 1;
	char ring[10000], *ring_ptr;
	uint32_t ring_size =  conf_get_number_value( CONF_ATT__MEMPOOL__SMP_RING_SIZE );
	uint32_t *rule_range, rule_id;
	uint32_t thread_id;
	size_t size;
	rule_info_t const*const*rules_array;

	size_t rules_count = mmt_sec_get_rules_info( &rules_array );
	ASSERT(rules_count > 0, "No rule to verify");

	//number of threads <= number of rules
	if( rules_count < threads_count ){
		log_write( LOG_WARNING, "Number of threads is greater than number of rules (%d > %zu). Use %zu threads.", threads_count, rules_count, rules_count );
		threads_count = rules_count;
	}

	mmt_smp_sec_handler_t *handler = mmt_mem_alloc( sizeof( mmt_smp_sec_handler_t ));

	handler->verbose       = verbose;
	handler->threads_count = threads_count;
	handler->rings         = mmt_mem_alloc( sizeof( void *) * handler->threads_count );

	handler->mmt_single_sec_handlers = mmt_mem_alloc( sizeof( mmt_single_sec_handler_t *) * handler->threads_count );

	//one ring per thread
	for( i=0; i<handler->threads_count; i++){
		handler->rings[ i ] = ring_init( ring_size );
		handler->mmt_single_sec_handlers[ i ] = NULL;
	}

	all_rules = rule_ptr = mmt_mem_dup( rules_array, rules_count * sizeof( void*));
	if( rule_mask != NULL ){
		for( i=0; i<handler->threads_count; i++ ){
			rules_count_per_thread = get_special_rules_for_thread( i+1, rule_mask, &rule_range );
			if( rules_count_per_thread == 0 )
				continue;

			//move spcial rules to the beginning
			for( j=0; j<rules_count_per_thread; j++ ){
				rule_id = rule_range[ j ];
				for( k=j; k<rules_count; k++ )
					if( rule_id == rule_ptr[k]->id ){
						//swap rule_ptr[j] and rule_ptr[k];
						tmp         = rule_ptr[j];
						rule_ptr[j] = rule_ptr[k];
						rule_ptr[k] = tmp;
						break;
					}

				//swap must be called one time
				ASSERT( k <= rules_count, "Rule mask is incorrect: rule %"PRIu32" does not exist.", rule_id );
			}

			if( verbose ){
				size = 0;
				for( j=0; j<rules_count_per_thread; j++ ){
					if( size > ring_len ) break;
					size += snprintf(ring + size, ring_len - size, "%"PRIu32"%c", rule_ptr[j]->id, j == rules_count_per_thread - 1? ' ':',' );
				}
				ring[size] = '\0';

				log_write( LOG_INFO,"Thread %2d processes %4d rules: %s", i + 1, rules_count_per_thread, ring );
			}

			handler->mmt_single_sec_handlers[ i ] = mmt_single_sec_register( rule_ptr, rules_count_per_thread, verbose, callback, user_data );

			rule_ptr      += rules_count_per_thread;
			rules_count   -= rules_count_per_thread; //number of remaining rules
			threads_count --;//number of remaining threads

			mmt_mem_free( rule_range );
		}
	}//end if( rule_mask != NULL )

	//each handler manages #rules_count_per_thread
	//e.g., if we have 10 rules and 3 threads
	// => each thread will manage 3 rules unless the last thread manages 4 rules
	for( i=0; i<handler->threads_count; i++ ){

		//this thread has been initiated above
		if( handler->mmt_single_sec_handlers[ i ] != NULL )
			continue;

		rules_count_per_thread = rules_count / threads_count;

		if( verbose){
			size = 0;
			for( j=0; j<rules_count_per_thread; j++ ){
				if( size > ring_len ) break;
				size += snprintf(ring + size, ring_len - size,"%"PRIu32"%c", rule_ptr[j]->id, j == rules_count_per_thread - 1? ' ':',' );
			}
			ring[size] = '\0';

			log_write( LOG_INFO,"Thread %2d processes %4d rules: %s", i + 1, rules_count_per_thread, ring );
		}

		handler->mmt_single_sec_handlers[ i ] = mmt_single_sec_register( rule_ptr, rules_count_per_thread, verbose, callback, user_data );
		rule_ptr    += rules_count_per_thread;
		rules_count -= rules_count_per_thread; //number of remaining rules
		threads_count --;//number of remaining threads
	}

	handler->threads_id = mmt_mem_alloc( sizeof( pthread_t ) * handler->threads_count );
	for( i=0; i<handler->threads_count; i++ ){
		thread_arg          = mmt_mem_alloc( sizeof( struct _thread_arg ));
		thread_arg->index   = i;
		thread_arg->handler = handler;
		if( core_mask == NULL )
			thread_arg->lcore = 0;
		else
			thread_arg->lcore   = core_mask[ i ];
		ret = pthread_create( &handler->threads_id[ i ], NULL, _process_one_thread, thread_arg );
		ASSERT( ret == 0, "Cannot create thread %d", (i+1) );
	}

	mmt_mem_force_free( all_rules );
	return handler;
}

void mmt_smp_sec_set_ignore_remain_flow( mmt_smp_sec_handler_t *handler, bool ignore, uint64_t buffer_size ){
	int i;
	for( i=0; i<handler->threads_count; i++ )
		mmt_single_sec_set_ignore_remain_flow( handler->mmt_single_sec_handlers[ i ], ignore, buffer_size );
}

bool mmt_smp_is_ignore_remain_flow( mmt_smp_sec_handler_t *handler, uint64_t flow_id ){
	int i;
	for( i=0; i<handler->threads_count; i++ ){
		bool ret = mmt_single_is_ignore_remain_flow( handler->mmt_single_sec_handlers[ i ], flow_id );
		if( ! ret )
			return false;
	}
	return true;
}

/**
 * Public API
 */
void mmt_smp_sec_process( mmt_smp_sec_handler_t *handler, message_t *msg ){
	int ret;
	lock_free_spsc_ring_t *ring;
	size_t total_retain = 0, i;
	//TODO: this limit 64 threads of mmt_smp_sec
	uint64_t mask;

#ifdef DEBUG_MODE
	ASSERT( handler != NULL, "handler cannot be null");
#endif

	//TODO: remove this
//	if( msg != NULL ){
//		free_message_t( msg );
//		return;
//	}

	//all threads does not receive receive message: turn on the first #threads_count bits
	mask =  (1LL << handler->threads_count) - 1;

	if( likely( msg != NULL )){
		//	DEBUG("%"PRIu64" verify rule", msg->counter );
		for( i=0; i<handler->threads_count; i++ ){
			//the message does not concern to any rules handled by this thread
			//==> do not need to push the message into the queue of this thread
			if( (msg->hash & handler->mmt_single_sec_handlers[i]->hash) == 0 )
				BIT_CLEAR( mask, i );
			else
				total_retain ++;
		}

		//no thread requires this message ?
		if( unlikely( total_retain == 0 )){
			free_message_t( msg );
			return;
		}

		//retain message for each thread
		//-1 since msg was cloned from message -> it has ref_count = 1
		//=> we need to increase ref_count only ( handler->threads_count - 1)
		msg = mmt_mem_retains( msg,  total_retain - 1 );
	}





	//still have one threads is not received msg
	while( mask != 0 ){
		for( i=0; i<handler->threads_count; i++ ){

			//if ring i-th has been put the message
			if( unlikely( BIT_CHECK( mask, i ) == 0 ))
				continue;

			//insert msg to a ring
			// if we cannot insert to the current ring (e.g., ring is full), we omit it and continue for next ring
			// then, go back to the current one after processing the last ring
			if( ring_push( handler->rings[ i ], msg ) == RING_SUCCESS )
				BIT_CLEAR( mask, i );
//#ifdef DEBUG_MODE
//			else
//				ring_wait_for_poping( handler->rings[ i ] );
//#endif
		}
		//DEBUG("ring full %"PRIu64, hash_index );
	}
}


#ifdef MODULE_ADD_OR_RM_RULES_RUNTIME

void mmt_smp_sec_add_rules( mmt_smp_sec_handler_t *handler, const char*rules_mask ){
	int i;
	size_t add_rules_count;
	uint32_t *new_rules_arr;
	size_t ret;
	for( i=0; i<handler->threads_count; i++ ){
		new_rules_arr = NULL;
		//get all rules if for this thread
		add_rules_count = get_special_rules_for_thread( i+1, rules_mask, &new_rules_arr);
		if( add_rules_count == 0 ){
			if( handler->verbose )
				log_write( LOG_INFO,"- Added %zu rule(s) to thread %d", add_rules_count, (i+1));
			mmt_mem_free( new_rules_arr );
			continue;
		}

		ret = mmt_single_sec_add_rules(handler->mmt_single_sec_handlers[ i ], add_rules_count, new_rules_arr);
		if( handler->verbose )
			log_write( LOG_INFO,"- Added %zu/%zu rule(s) to thread %d", ret, add_rules_count, (i+1));

		//#get_special_rules_for_thread create a new memory => we need to free it
		mmt_mem_free( new_rules_arr );
	}
}
void mmt_smp_sec_remove_rules( mmt_smp_sec_handler_t *handler ){
	int i;
	size_t rm_rules_count, old_rules_count;
	for( i=0; i<handler->threads_count; i++ ){
		old_rules_count = handler->mmt_single_sec_handlers[i]->rules_count;

		if( handler->verbose)
			log_write( LOG_INFO, "- Removing rules from thread %d", (i+1) );
		rm_rules_count = mmt_single_sec_remove_rules( handler->mmt_single_sec_handlers[i] );

		if( handler->verbose)
			printf( " => %zu/%zu rule(s) being removed\n", rm_rules_count, old_rules_count);
	}
}

#endif
