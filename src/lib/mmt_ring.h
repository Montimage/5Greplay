/*
 * ring.h
 *
 *  Created on: Jun 7, 2018
 *          by: Huu Nghia Nguyen
 *
 * A simple implementation of buffer ring.
 * The elements of ring can be anything, (void *) by default.
 * To use other data type, one need to override MMT_RING_ELEMENT_TYPE.
 * For example:
 *  #define MMT_RING_ELEMENT_TYPE uint32_t
 *  #include "mmt_ring.h"
 *
 */

#ifndef MMT_RING_H_
#define MMT_RING_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifndef MMT_RING_ELEMENT_TYPE
#define MMT_RING_ELEMENT_TYPE void *
#endif

typedef struct {
	uint32_t head;
	uint32_t tail;
	uint32_t size;
	MMT_RING_ELEMENT_TYPE buffer[];
} mmt_ring_t;

/**
 * Create a new ring
 * @param size is number of elements the ring can contain
 * @return
 * - a ring if OK
 * - NULL if no memory
 */
static inline mmt_ring_t* mmt_ring_create( uint32_t size ){
	if( size == 0 )
		return NULL;

	//size of an element
	size_t elem_size = sizeof( MMT_RING_ELEMENT_TYPE );

	//memory for the ring and its elements
	mmt_ring_t *ring = malloc( sizeof( mmt_ring_t) + (elem_size * size ));

	if(ring == NULL )
		return NULL;

	ring->head = 0;
	ring->tail = 0;
	ring->size = size;

	return ring;
}

/**
 * Free the ring
 * @param ring
 */
static inline void mmt_ring_free( mmt_ring_t *ring ){
	free( ring );
}

static inline bool mmt_ring_is_full( mmt_ring_t * ring ){
	// We determine "full" case by head being one position behind the tail
	// Note that this means we are wasting one space in the buffer!
	uint32_t next_pos = (ring->head + 1) % ring->size;
    if( next_pos == ring->tail) //ring is full
    	return true;
    return false;
}

/**
 * Enqueue an element to the ring
 * @param ring
 * @param data
 * @return
 *  - true if OK
 *  - false if ring is full
 */
static inline bool mmt_ring_enqueue(mmt_ring_t * ring, MMT_RING_ELEMENT_TYPE data){
	uint32_t next_pos = (ring->head + 1) % ring->size;

    if( next_pos == ring->tail)//ring is full
    	return false;

    //put data to head
	ring->buffer[ ring->head ] = data;
	ring->head = next_pos;

	return true;
}

static inline int mmt_ring_is_empty(mmt_ring_t * ring ){
	return ( ring->head == ring->tail );
}

/**
 * Dequeue an element from the ring
 * @param ring
 * @param data
 * @return
 * - true if OK
 * - false if ring is empty
 */
static inline int mmt_ring_dequeue(mmt_ring_t * ring, MMT_RING_ELEMENT_TYPE *data){
	//ring is empty
	if( ring->head == ring->tail )
		return false;

	*data = ring->buffer[ring->tail];
	ring->tail = (ring->tail + 1) % ring->size;

	return true;
}

/**
 * Reset the ring
 * @param ring
 */
static inline void mmt_ring_reset( mmt_ring_t *ring ){
	if( ring == NULL  )
		return;
	ring->head = 0;
	ring->tail = 0;
}
#endif /* MMT_RING_H_ */
