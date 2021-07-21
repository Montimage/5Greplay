/*
 * lock_free_spsc_ring.c
 *
 *  Created on: 31 mars 2016
 *      Author: nhnghia
 *
 * An implementation of Lamport queue without lock
 * based on https://github.com/blytkerchan/Lamport
 *
 * Modified on 24 nov. 2016
 */
#include <stdlib.h>
#include <unistd.h>
#include <stdatomic.h>
#include "mmt_lib.h"
#include "lock_free_spsc_ring.h"
#include "valgrind.h"

void ring_free( lock_free_spsc_ring_t *q ){
	if( q == NULL ) return;
	if( q->_data ) mmt_mem_free( q->_data );
	#ifdef SPIN_LOCK
		pthread_spin_destroy( &(q->spin_lock) );
	#endif

	mmt_mem_free( q );
}

lock_free_spsc_ring_t* ring_init( uint32_t size ){
	lock_free_spsc_ring_t *q = mmt_mem_alloc( sizeof( lock_free_spsc_ring_t ));

	q->_data = mmt_mem_alloc( sizeof( void *) * size );
	q->_size = size;

	q->_head = q->_tail = 0;
	q->_cached_head = q->_cached_tail = 0;

	EXEC_ONLY_IN_VALGRIND_MODE( DRD_IGNORE_VAR( q )  );
	EXEC_ONLY_IN_VALGRIND_MODE( DRD_IGNORE_VAR( q->_size )  );
	EXEC_ONLY_IN_VALGRIND_MODE( DRD_IGNORE_VAR( q->_head )  );
	EXEC_ONLY_IN_VALGRIND_MODE( DRD_IGNORE_VAR( q->_tail )  );
	EXEC_ONLY_IN_VALGRIND_MODE( DRD_IGNORE_VAR( q->_cached_head )  );
	EXEC_ONLY_IN_VALGRIND_MODE( DRD_IGNORE_VAR( q->_cached_tail )  );
//	sem_init( &q->sem_wait_pushing, 0, 0 );

	return q;
}
