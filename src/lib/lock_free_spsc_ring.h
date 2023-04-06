/*
 * lock_free_spsc_ring.c
 *
 *  Created on: 31 mars 2016
 *      Author: nhnghia
 */

#ifndef SRC_LOCK_FREE_SPSC_RING_H_
#define SRC_LOCK_FREE_SPSC_RING_H_

#include <stdint.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdatomic.h>
#include <time.h>
#include <semaphore.h>
#include "mmt_lib.h"
#include "valgrind.h"

//this lib needs to be compiled by gcc >= 4.9
#define GCC_VERSION (__GNUC__ * 10000        \
                     + __GNUC_MINOR__ * 100  \
                     + __GNUC_PATCHLEVEL__)

// Test for GCC < 4.9.0
#if GCC_VERSION < 40900
	#warning Need gcc >= 4.9
#endif

#define RING_EMPTY  -1
#define RING_FULL   -2
#define RING_SUCCESS 0

typedef struct lock_free_spsc_ring_struct
{
    volatile uint32_t _head __aligned;
    volatile uint32_t _tail;

    uint32_t _cached_head;
    uint32_t _cached_tail;

    uint32_t _size;

    void **_data;

    //pthread_mutex_t mutex_wait_pushing, mutex_wait_poping;
    //pthread_cond_t cond_wait_pushing, cond_wait_poping;
    //sem_t sem_wait_pushing, sem_wait_poping;

}lock_free_spsc_ring_t;

#ifdef atomic_load_explicit
#undef atomic_load_explicit
#endif

#ifdef atomic_store_explicit
#undef atomic_store_explicit
#endif

#define atomic_load_explicit( x, y )    __sync_fetch_and_add( x, 0 )
#define atomic_store_explicit( x, y, z) __sync_lock_test_and_set( x, y)
//#define atomic_store_explicit( x, y, z) __sync_synchronize( *x = y )
/**
 * Create a circular buffer. This is thread-safe only in when there is one
 * producer and one consumer that are 2 different threads.
 * The producer accesses only the functions: #ring_push and #ring_wait_for_poping,
 * meanwhile the consumer accesses only the functions: #ring_pop and #ring_wait_for_pushing.
 * - Input:
 * 	+ size: buffer size
 * - Return:
 * 	- buffer
 */
lock_free_spsc_ring_t* ring_init( uint32_t size );

/**
 * Push a pointer to the buffer.
 * This function can be called only by producer.
 * - Input:
 * 	+ q: the buffer to be pushed
 * 	+ val:
 * - Return:
 * 	+ RING_SUCESS if #val was successfully pushed into buffer
 * 	+ RING_FULL if the buffer is full, thus the #val is not inserted
 * 	If one stills want to insert #val, thus need to call #ring_wait_for_poping then
 * 	try to push again by calling #ring_push
 */
static inline int  ring_push( lock_free_spsc_ring_t *q, void* val  ){
	uint32_t h = q->_head;

	//I always let 1 available elements between head -- tail
	//it is reading by the consumer
	if( ( h + 2 ) % ( q->_size ) == q->_cached_tail ){
		q->_cached_tail = atomic_load_explicit( &q->_tail, memory_order_acquire );

	/* tail can only increase since the last time we read it, which means we can only get more space to push into.
		 If we still have space left from the last time we read, we don't have to read again. */
		if( ( h + 2 ) % ( q->_size ) == q->_cached_tail )
			return RING_FULL;
	}
	//not full
	q->_data[ h ] = val;
	//EXEC_ONLY_IN_VALGRIND_MODE(ANNOTATE_HAPPENS_BEFORE( & (q->_data[h] )));
	EXEC_ONLY_IN_VALGRIND_MODE(ANNOTATE_HAPPENS_BEFORE( &(q->_data) ));

	atomic_store_explicit( &q->_head, (h +1) % q->_size, memory_order_release );

//	sem_post( &q->sem_wait_pushing );

	return RING_SUCCESS;
}

static inline int  ring_push_burst( lock_free_spsc_ring_t *q, size_t count, void** array  ){
	uint32_t h;
	int i;
	h = q->_head;

	//I always let 1 available elements between head -- tail
	//that is the element being reading by the consumer
	if( ( h + 2 ) % ( q->_size ) == q->_cached_tail ){
		q->_cached_tail = atomic_load_explicit( &q->_tail, memory_order_acquire );

	/* tail can only increase since the last time we read it, which means we can only get more space to push into.
		 If we still have space left from the last time we read, we don't have to read again. */
		if( ( h + 2 ) % ( q->_size ) == q->_cached_tail )
			return RING_FULL;
	}

	//not full
	for( i = 0; i<count && ((h + i + 1) % q->_size != q->_cached_tail); i++ )
		q->_data[ h + i ] = array[ i ];

	atomic_store_explicit( &q->_head, (h + i) % q->_size, memory_order_release );

//	sem_post( &q->sem_wait_pushing );

	return count-i;
}

/**
 * Pop an element of buffer.
 * This function can be called only by consumer.
 * - Input:
 * 	+ q: ring to pop
 * - Output:
 * 	+ val: point to the result element if success
 * - Return:
 * 	+ RING_SUCCESS if everything is OK
 * 	+ RING_EMPTY if the buffer is empty
 */

static inline int  ring_pop ( lock_free_spsc_ring_t *q, void **val ){
	uint32_t  t;
	t = q->_tail;

	if( q->_cached_head == t )
		q->_cached_head = atomic_load_explicit ( &q->_head, memory_order_acquire );

	 /* head can only increase since the last time we read it, which means we can only get more items to pop from.
		 If we still have items left from the last time we read, we don't have to read again. */
	if( q->_cached_head == t )
		return RING_EMPTY;
	else{
		//not empty
		*val = q->_data[ t ];

		atomic_store_explicit( &q->_tail, (t+1) % q->_size, memory_order_release );

		return RING_SUCCESS;
	}
}

/**
 * Pop all elements of buffer.
 * This function can be called only by consumer
 * - Input:
 * 	+ q: ring to pop
 * 	+ length: maximum number of elements to pop
 * 	+ array: array of pointers to contain results.
 * 				This array must have at least #length elements
 * - Ouput:
 * 	+ array: array of pointers points to data
 * - Return:
 * 	- number of elements popped successfully
 * 		This number must be less than or equal to #length
 */
static inline size_t ring_pop_burst( lock_free_spsc_ring_t *q, int length, void **array ){
	int size;
	uint32_t t = q->_tail;

	if( q->_cached_head == t ){
		q->_cached_head = atomic_load_explicit ( &q->_head, memory_order_acquire );

	 /* head can only increase since the last time we read it, which means we can only get more items to pop from.
		 If we still have items left from the last time we read, we don't have to read again. */
		if( q->_cached_head == t ) return 0;
	}

	//not empty
	//this condition ensures that we get a continues memory segment
	//=> to use memcpy
	if( q->_cached_head > t ){
		size = q->_cached_head - t;
	}else{
		size = q->_size - t;
	}

	//check limit
	if( unlikely( size > length ))
		size = length;

	//EXEC_ONLY_IN_VALGRIND_MODE( ANNOTATE_HAPPENS_AFTER( & (q->_data[t] )));
	EXEC_ONLY_IN_VALGRIND_MODE( ANNOTATE_HAPPENS_AFTER( &(q->_data) ));
	//copy result
	memcpy( array, &(q->_data[t]), size * sizeof( void *) );

	//seek tail of ring to the new position
	atomic_store_explicit( &q->_tail, (t + size) % q->_size, memory_order_release );

	return size;
}

/**
 * Free a buffer.
 * This function frees resource using by the buffer and also the pointer #q
 */
void ring_free( lock_free_spsc_ring_t *q );

/**
 *
 */
static inline void ring_wait_for_pushing( lock_free_spsc_ring_t *q ){
	nanosleep( (const struct timespec[]){{0, 5000L}}, NULL );
//	if( unlikely( sem_trywait( &q->sem_wait_pushing) == 0 ))
//		return; //already lock
//	else{
//		sem_wait( &q->sem_wait_pushing );
//	}
}


static inline void ring_wait_for_poping( lock_free_spsc_ring_t *q ){
	nanosleep( (const struct timespec[]){{0, 100L}}, NULL );
}

#endif /* SRC_QUEUE_LOCK_FREE_SPSC_RING_H_ */
