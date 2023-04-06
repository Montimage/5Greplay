/*
 * spsc.c
 *
 *  Created on: Nov 22, 2016
 *  Created by: Huu Nghia NGUYEN <huunghia.nguyen@montimage.com>
 */

#include <locale.h>
#include <pthread.h>
#include <sys/time.h>
#include "../src/lib/mmt_lib.h"
#include "../src/lib/lock_free_spsc_ring.h"
#include "../src/lib/system_info.h"

typedef struct _user_data_struct{
	lock_free_spsc_ring_t *ring;
	size_t index;
	size_t total;
} _user_data_t;


void *_consumer_fn( void *arg ){
	size_t total = 0, i;
	_user_data_t *u_data = (_user_data_t * ) arg;

	lock_free_spsc_ring_t *ring = u_data->ring;
	void *arr[10];
	int ret;

	size_t total_cpu = get_number_of_online_processors();
	size_t cpu_index = u_data->index;
	if( total_cpu > 1 ){
	if( cpu_index >= total_cpu )
		cpu_index = cpu_index % (total_cpu - 1) + 1; //the first CPU is reserved for the main thread
		if( move_the_current_thread_to_a_processor( cpu_index, -10  ) )
			log_write( LOG_WARNING,"Cannot move the thread %2zu (pid = %d) to cpu[%zu]", u_data->index, gettid(), cpu_index );
	}

	do{
		ret = ring_pop_burst( ring, 10, arr );
		if( unlikely( ret == 0 ))
			ring_wait_for_pushing( NULL );
		else{
			for( i=0; i<ret; i++ )
				total += (size_t) arr[i];
			//last msg
			if( arr[ret-1] == NULL )
				break;
		}
	}while( 1 );


	VALGRIND_MODE(ANNOTATE_IGNORE_READS_BEGIN());
//	log_write( LOG_INFO,"Thread %2zu (pid = %d): total from 1 to %'zu = %'zu",
//			u_data->index,
//			gettid(),
//			u_data->total, total );
	mmt_mem_free( u_data );
	VALGRIND_MODE(ANNOTATE_IGNORE_READS_END());
	return NULL;
}

int main( int argc, char **args ){
	size_t consumers_count = 2;
	size_t loops_count     = 10*1000*1000;
	size_t ring_size       = 1000;
	size_t i,j;
	lock_free_spsc_ring_t **rings;
	pthread_t *p_ids;
	int ret;
	_user_data_t *u_data;
	struct timeval start_time, end_time;

	if( argc > 1 )
		consumers_count = atoll( args[ 1 ] );
	if( argc > 2 )
		loops_count     = atoll( args[ 2 ] );
	if( argc > 3 )
		ring_size       = atoll( args[ 3 ] );
	log_write( LOG_INFO, "Usage: %s consumers_count loops_count ring_size", args[ 0 ] );
	log_write( LOG_INFO, "Number of online lcores: %ld", get_number_of_online_processors() );
	setlocale(LC_ALL,"en_US.UTF-8");
	log_write( LOG_INFO, "Running %'zu loops with 1 producer and %'zu consumers. Size of each ring is %'zu",
			loops_count, consumers_count, ring_size );

	rings = mmt_mem_alloc( sizeof( void *) * consumers_count );
	for( i=0; i< consumers_count; i++ )
		rings[ i ] = ring_init( ring_size );

	p_ids = mmt_mem_alloc( sizeof( pthread_t ) * consumers_count );

	time_t start = time(NULL);
	for( i=0; i< consumers_count; i++ ){
		VALGRIND_MODE(ANNOTATE_IGNORE_WRITES_BEGIN());
		u_data = mmt_mem_alloc( sizeof( _user_data_t) );
		u_data->ring  = rings[ i ];
		u_data->index = i;
		u_data->total = loops_count;
		VALGRIND_MODE(ANNOTATE_IGNORE_WRITES_END());
		ASSERT( pthread_create( &(p_ids[i]), NULL, _consumer_fn, u_data ) == 0,
				"Cannot create thread %zu", i );
	}

	//fix the main thread on cpu[0]
	if( move_the_current_thread_to_a_processor( 0, -15 ) )
			log_write( LOG_WARNING,"Cannot move the current thread to cpu[0]");

	gettimeofday( &start_time, NULL );
	//producer
	for( j=1; j<loops_count; j++ ){
		for( i=0; i< consumers_count; i++ ){
			do{
				ret = ring_push( rings[ i ], (void *) (j + 1) );
				if( likely( ret == RING_SUCCESS ))
					break;
				else
					ring_wait_for_poping( NULL );
			}while( 1 );
		}
	}

	//insert NULL to exit consumers
	for( i=0; i< consumers_count; i++ ){
		do{
			ret = ring_push( rings[ i ], NULL );
			if( likely( ret == RING_SUCCESS ))
				break;
			else
				ring_wait_for_poping( NULL );
		}while( 1 );
	}

	//waiting for the consumers finish
	for( i=0; i< consumers_count; i++ )
		pthread_join( p_ids[i], NULL );

	gettimeofday( &end_time, NULL );

	printf("%.2f seconds \n", (end_time.tv_usec - start_time.tv_usec) / 1000000.0f + (double)(end_time.tv_sec - start_time.tv_sec) );

	for( i=0; i< consumers_count; i++ )
		ring_free( rings[ i ] );
	mmt_mem_free( rings );
	mmt_mem_free( p_ids );
	return 0;
}

