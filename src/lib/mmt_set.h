/*
 * mmt_set.h
 *
 *  Created on: Sep 7, 2018
 *          by: Huu Nghia Nguyen
 * This file implements a set containing elements of any data type.
 * The set supports basically 3 operations:
 * (1) add an element.
 * (2) remove an element.
 * (3) check whether the set contains an element.
 *
 * By default, an element is uint64_t
 * To change the type, one need to override MMT_SET_ELEMENT_TYPE
 *
 * For example:
 *  #define MMT_SET_ELEMENT_TYPE uint32_t
 *  #include "mmt_set.h"
 *
 * One may need also to override MMT_SET_ELEMENT_EMPTY_VALUE to define a special value.
 * The element must not use this value. It is used to mark a slot being available.
 *
 * One may need also to override MMT_SET_HASH_ELEMENT_VALUE to define a hash function to get
 * a good distribution of key. The hash function must return a number.
 *
 * Bellow are an example applicable for uint32_t numbers:
 *
 * //http://stackoverflow.com/a/12996028/1069256
 * static inline uint32_t hash(uint32_t x) {
 *    x = ((x >> 16) ^ x) * 0x45d9f3b;
 *    x = ((x >> 16) ^ x) * 0x45d9f3b;
 *    x = (x >> 16) ^ x;
 *    return x;
 * }
 *
 * #define MMT_SET_HASH_ELEMENT_VALUE(i) hash(i)
 *
 */

#ifndef SRC_LIB_MMT_SET_H_
#define SRC_LIB_MMT_SET_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <errno.h>

#ifndef MMT_SET_ELEMENT_TYPE
#define MMT_SET_ELEMENT_TYPE uint64_t
#endif

//a special value user cannot add to the set
#ifndef MMT_SET_ELEMENT_EMPTY_VALUE
#define MMT_SET_ELEMENT_EMPTY_VALUE (-1)
#endif

//Knuth's multiplicative method:
#ifndef MMT_SET_HASH_ELEMENT_VALUE
#define MMT_SET_HASH_ELEMENT_VALUE( i ) ( (i)*2654435761 >> 8 )
#endif


/**
 * MMT Set table
 */
typedef struct mmt_set_struct{
	uint32_t size;   //number of elements the set can contain
	uint32_t count;  //number of actual elements in the set
	MMT_SET_ELEMENT_TYPE data[];
}mmt_set_t;


/**
 * Create a new set
 * @param size is the number of elements the set can contain
 */
static inline mmt_set_t* mmt_set_create( uint32_t size ){
	size_t i;

	//size of an element
	size_t elem_size = sizeof( MMT_SET_ELEMENT_TYPE );

	//allocate memory for the set
	mmt_set_t *ret = malloc( sizeof( mmt_set_t ) + (elem_size * size ));
	if( ret == NULL )
		return NULL;

	ret->size  = size;
	ret->count = 0;

	//empty all element slots
	for( i=0; i<ret->size; i++ )
		ret->data[i]  = MMT_SET_ELEMENT_EMPTY_VALUE;

	return ret;
}

static inline void mmt_set_free( mmt_set_t *hash ){
	free( hash );
}

static inline bool mmt_set_is_full( const mmt_set_t *set ){
	return ( set->count == set->size );
}

static inline bool mmt_set_is_empty( const mmt_set_t *set ){
	return ( set->count == 0 );
}
/**
 * Add a new element to the set
 * @param set
 * @param elem
 * @return
 * - -EINVAL if the parameters are invalid.
 * - -ENOSPC if there is no space in the set for this element.
 * - -EEXIST if there exist the element in the set
 * - index of the slot in the set to store the element
 */
static inline int64_t mmt_set_add( mmt_set_t *set, MMT_SET_ELEMENT_TYPE elem){
	//the element must not use the special value
	if( elem == MMT_SET_ELEMENT_EMPTY_VALUE )
		return (-EINVAL);

	uint32_t index   = MMT_SET_HASH_ELEMENT_VALUE( elem) % set->size;
	uint32_t counter = 0;

	//find an available slot
	while( set->data[ index ] != MMT_SET_ELEMENT_EMPTY_VALUE ){

		if( likely( set->data[ index ] == elem ))
			return index;

		//go to the next slot
		counter ++;
		index ++;

		//find all table but not found any empty slot
		if( unlikely( counter >= set->size ))
			return (-ENOSPC);

		//return to zero if it goes over
		index %= set->size;
	}

	//assign the new element to this slot
	set->data[ index ] = elem;
	return index;
}

/**
 * Check whether the set contain an element
 * @param set
 * @param elem
 * - -EINVAL if the parameters are invalid.
 * - -ENOENT if the element is not found.
 * - index of the element being stored in the set
 */
static inline int64_t mmt_set_check( const mmt_set_t *set, MMT_SET_ELEMENT_TYPE elem ){
	//the element must not use the special value
	if( elem == MMT_SET_ELEMENT_EMPTY_VALUE )
		return (-EINVAL);

	uint32_t index   = MMT_SET_HASH_ELEMENT_VALUE(elem) % set->size;
	uint32_t counter = 0;

	//find an available slot
	while( set->data[ index ] != MMT_SET_ELEMENT_EMPTY_VALUE ){

		if( likely( set->data[ index ] == elem ))
			return index;

		//go to the next slot
		counter ++;
		index ++;

		//find all table but not found
		if( unlikely( counter >= set->size ))
			return (- ENOENT);

		//return to zero if it goes over
		index %= set->size;
	}

	return (- ENOENT);
}

/**
 * Empty a slot in the set
 * @param set
 * @param slot_index
 * @return
 * - false if the slot_index is not valid
 * - true if OK
 */
static inline bool mmt_set_empty_slot( mmt_set_t *set, uint32_t slot_index){
	if( slot_index >= set->size )
		return false;
	set->data[ slot_index ] = MMT_SET_ELEMENT_EMPTY_VALUE;
	return true;
}

#endif /* SRC_LIB_MMT_SET_H_ */
