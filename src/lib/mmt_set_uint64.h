/*
 * mmt_set_ex.h
 *
 *  Created on: Sep 7, 2018
 *          by: Huu Nghia Nguyen
 *
 * This file override the implementation of mmt_set by adding a new feature:
 *  When the set is full, the oldest element will be removed to get a place to store a new element.
 *  The oldest element is the one that being enqueued earliest.
 *
 * The set use mmt_set_t to store its elements.
 * It use also a ring to store the index of slots containing elements.
 *
 * A filter is used to quickly check if a number does not exist in the set.
 */

#ifndef SRC_LIB_MMT_SET_UINT64_H_
#define SRC_LIB_MMT_SET_UINT64_H_

#define MMT_SET_ELEMENT_TYPE        uint64_t
#define MMT_SET_ELEMENT_EMPTY_VALUE ((uint64_t) (-1))
#include "mmt_set.h"

//as the ring is used to store indexes of an array => uint32_t is enough
#define MMT_RING_ELEMENT_TYPE uint32_t
#include "mmt_ring.h"

#include "mmt_filter.h"

typedef struct{
	mmt_set_t  *set;
	mmt_ring_t *ring;
	mmt_filter64_t *filter;
}mmt_set_ex_t;

static inline void mmt_set_ex_free( mmt_set_ex_t *set ){
	if( set ){
		if( set->set )
			mmt_set_free( set->set );
		if( set->ring)
			mmt_ring_free( set->ring );
		if( set->filter )
			mmt_filter64_free( set->filter);
	}
	free( set );
}

/**
 * Create a new set
 * @param size is the number of elements the set can contain
 */
static inline mmt_set_ex_t* mmt_set_ex_create( uint32_t size ){
	//allocate memory for the set
	mmt_set_ex_t *ret = malloc( sizeof( mmt_set_ex_t ) );
	if( ret == NULL )
		return NULL;

	//create its set
	ret->set = mmt_set_create( size );
	if( ret->set == NULL ){
		free( ret );
		return NULL;
	}

	//create its ring
	ret->ring = mmt_ring_create( size );
	if( ret->ring == NULL ){
		mmt_set_free( ret->set );
		free( ret );
		return NULL;
	}

	//create its filter
	ret->filter = mmt_filter64_create();
	if( ret->filter == NULL ){
		mmt_set_free( ret->set );
		mmt_ring_free( ret->ring );
		free( ret );
		return NULL;
	}

	return ret;
}

/**
 * Add a new element to the set
 * @param set
 * @param elem
 * @return
 * - true if OK
 * - false, otherwise
 */
static inline bool mmt_set_ex_add( mmt_set_ex_t *set, uint64_t elem){
	//the element must not use the special value
	if( elem == MMT_SET_ELEMENT_EMPTY_VALUE )
		return false;

	if( mmt_ring_is_full( set->ring ) ){
		//get the oldest element
		uint32_t index = 0;

		//to increase the performance, we will several elements.
		//the number of elements to be removed = 1/16 total numbers of elements
		int nb_to_rm = 1;
		if( set->ring->size > 16 )
			nb_to_rm = set->ring->size >> 4; // 1/16

		// do a burst
		while( nb_to_rm > 0 ){
			nb_to_rm --;
			//get index of the oldest element
			mmt_ring_dequeue( set->ring, &index );

			//remove the element from the filter
			uint64_t oldest_elem = set->set->data[ index ];
			mmt_filter64_rm( set->filter, oldest_elem );

			//free the slot containing the element
			mmt_set_empty_slot( set->set, index );
		}
	}

	int64_t index = mmt_set_add( set->set, elem );
	//if we add successfully the element to the set, then
	// we need to remember the index of the slot containing the element.
	// the index is used to decided the oldest element to be removed when the set is full
	if( index >= 0 ){
		//this may not happen when the set already contains the elem
		mmt_ring_enqueue( set->ring, (uint32_t) index );

		//add the number to the filter
		mmt_filter64_add( set->filter, elem );
	}

	return true;
}

/**
 * Check whether the set contain an element
 * @param set
 * @param elem
 * - true if the set contains the element
 * - false, otherwise
 */
static inline bool mmt_set_ex_check( const mmt_set_ex_t *set, uint64_t elem ){
	//to increase the performance, we check the filter firstly
	// when the number doest not exist in the filter, we are sure that it does not exist also in the set
	if( ! mmt_filter64_check( set->filter, elem ) )
		return false;

	return (mmt_set_check( set->set, elem) >= 0 );
}

#endif /* SRC_LIB_MMT_SET_UINT64_H_ */
