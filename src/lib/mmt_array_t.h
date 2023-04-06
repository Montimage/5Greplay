/*
 * mmt_array_t.h
 *
 *  Created on: Nov 8, 2016
 *  Created by: Huu Nghia NGUYEN <huunghia.nguyen@montimage.com>
 */

#ifndef SRC_LIB_MMT_ARRAY_T_H_
#define SRC_LIB_MMT_ARRAY_T_H_

#include "mmt_alloc.h"

typedef struct mmt_array_struct{
	size_t elements_count;
	void **data;
}mmt_array_t;

/**
 * Create an array of #elements_count elements. Each element is a void*
 */
static inline __attribute__((always_inline))
mmt_array_t* mmt_array_init( size_t elements_count ){
	size_t i;
	mmt_array_t *ret = mmt_mem_alloc( sizeof( mmt_array_t) + elements_count * sizeof( void* ) );

	ret->elements_count = elements_count;
	ret->data           = (void *) (ret + 1);

	for( i=0; i<ret->elements_count; i++ )
		ret->data[ i ] = NULL;

	return ret;
}

static inline __attribute__((always_inline))
mmt_array_t* mmt_array_clone( const mmt_array_t *array, void* (*clone_data_fn)(void *) ){
	size_t i;
//	mmt_array_t *ret = mmt_array_init( array->elements_count );
	mmt_array_t *ret = mmt_mem_alloc( sizeof( mmt_array_t) + array->elements_count * sizeof( void* ) );
	ret->elements_count = array->elements_count;
	ret->data           = (void *) (ret + 1);

	if( clone_data_fn != NULL ){
		for( i=0; i<array->elements_count; i++ )
			ret->data[i] = clone_data_fn( array->data[ i ] );
	}else{
		for( i=0; i<array->elements_count; i++ )
			ret->data[i] = array->data[ i ];
	}
	return ret;
}

static inline __attribute__((always_inline))
mmt_array_t* mmt_array_alter_elements( mmt_array_t *array, void* (*alter_data_fn)(void *) ){
	size_t i;

	if( alter_data_fn != NULL )
		for( i=0; i<array->elements_count; i++ )
			array->data[i] = alter_data_fn( array->data[ i ] );

	return array;
}

static inline __attribute__((always_inline))
void mmt_array_free( mmt_array_t *array, void (*free_data_fn)(void *) ){
	size_t i;
	if( free_data_fn != NULL )
		for( i=0; i<array->elements_count; i++ )
			free_data_fn( array->data[ i ] );

	mmt_mem_force_free( array );
}

#endif /* SRC_LIB_MMT_ARRAY_T_H_ */
