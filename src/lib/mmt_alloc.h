/*
 * mmt_alloc.h
 *
 *  Created on: 19 sept. 2016
 *  Created by: Huu Nghia NGUYEN <huunghia.nguyen@montimage.com>
 *
 *  A wrapper for malloc, free, calloc, realloc
 *  By using this wrapper, we can know how much memory are allocated/free
 */

#ifndef SRC_MMT_ALLOC_H_
#define SRC_MMT_ALLOC_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "base.h"
#include "mmt_alloc.h"
#include "mmt_log.h"
#include "valgrind.h"

typedef struct mmt_memory_struct{
	uint32_t  ref_count;
	uint32_t  size;
	void*     data;
}mmt_memory_t;

#define SIZE_OF_MMT_MEMORY_T sizeof( mmt_memory_t )

#define mmt_mem_revert( x ) (mmt_memory_t *) ( (uint8_t*)x - SIZE_OF_MMT_MEMORY_T )

/**
 * A wrapper of malloc
 * Allocate a new segment of memory having the given size.
 * The segment is appended an extra byte containing by '\0'
 * - Input:
 * 	+ size: size to be allocated
 * - Output:
 * - Return:
 * 	+ new segment allocated
 * - Error:
 * 	+ Exit system if memory is not enough
 */
void *mmt_mem_alloc(size_t size);

static inline
void *mmt_mem_alloc_and_init_zero( size_t size ){
	void *data = mmt_mem_alloc(size);
	if( data )
		memset( data, 0, size );
	return data;
}

/**
 * Free memory allocated by mmt_malloc
 * Do not use this function to free memory created by malloc
 * @param x
 */
void mmt_mem_force_free( void *x );


/**
 * Free memory allocated by mmt_malloc
 * Do not use this function to free memory created by malloc
 */
static inline
size_t mmt_mem_free( void *x ){
	__check_null( x, 0);

   mmt_memory_t *mem = mmt_mem_revert( x );
   if( mem->ref_count <= 1 ){
   	mmt_mem_force_free( x );
		return 0;
   }else{
   	mem->ref_count --;
   	return mem->ref_count;
   }
}



/**
 * Get size of the memory segment pointed by ptr.
 * Note that ptr is the pointer created by the function: mmt_mem_alloc, mmt_mem_dup
 * - Error:
 * 	+ Maybe crashed if ptr is not created by mmt_mem_alloc, mmt_mem_dup
 */
static inline
size_t mmt_mem_size( const void *x ){
	__check_null( x, 0 );  // nothing to do

   mmt_memory_t *mem = mmt_mem_revert( x );
   return mem->size;
}

static inline
void* mmt_mem_force_dup( const void *ptr, size_t size ){
	void *ret = mmt_mem_alloc( size );
	memcpy( ret, ptr, size );
	return ret;
}


/**
 * Duplicate a memory
 * - Input:
 * 	+ ptr: data to be duplicated
 * 	+ size: size of data
 * - Output:
 * - Return
 * 	+ new data being duplicated
 */
static inline
void* mmt_mem_dup( const void *ptr, size_t size ){
	__check_null( ptr, NULL );  // nothing to do
	if( unlikely( size == 0)) return NULL;

	return mmt_mem_force_dup( ptr, size );
}

static inline
char * mmt_strdup( const char *str ){
	return mmt_mem_dup( str, strlen(str) );
}

/**
 * Increase number of reference to the memory to 1
 * - Input:
 * 	+ ptr: data to be increase
 * - Return:
 * 	a pointer point to #ptr;
 */
static inline
void *mmt_mem_retain( void *x ){
	__check_null( x, NULL );  // nothing to do
   mmt_memory_t *mem = mmt_mem_revert( x );
   mem->ref_count ++;
   return mem->data;
}

/**
 * Atomic version of #mmt_mem_retain
 * @param x
 */
static inline
void *mmt_mem_atomic_retain( void *x ){
	__check_null( x, NULL );  // nothing to do
   mmt_memory_t *mem = mmt_mem_revert( x );
   __sync_add_and_fetch( &mem->ref_count, 1 );
   return mem->data;
}

/**
 * Increase references of variable #retains_count times
 * @param x
 * @param retains_count
 */
static inline
void *mmt_mem_retains( void *x, size_t retains_count ){
	__check_null( x, NULL );  // nothing to do
   mmt_memory_t *mem = mmt_mem_revert( x );
   mem->ref_count += retains_count;
   return mem->data;
}

/**
 * Atomic version of #mmt_mem_retains
 * @param x
 * @param retains_count
 */
static inline
void *mmt_mem_atomic_retains( void *x, size_t retains_count ){
	__check_null( x, NULL );  // nothing to do
   mmt_memory_t *mem = mmt_mem_revert( x );
   __sync_add_and_fetch( &mem->ref_count, retains_count );
   return mem->data;
}


/**
 * Return number of pointers pointing to this memory
 */
static inline
size_t mmt_mem_reference_count( void *x ){
	if( x == NULL ) return 0; // nothing to do
	mmt_memory_t *mem = mmt_mem_revert( x );
	return mem->ref_count;
}

static inline int mmt_mem_cmp( const void *x, const void *y){
	int ret;
	__check_null( x, -1 );
	__check_null( y,  1 );

	mmt_memory_t *mx = mmt_mem_revert( x );
	mmt_memory_t *my = mmt_mem_revert( y );
	ret = mx->size - my->size;

	if( ret != 0 )
		return ret;
	else
		return memcmp( mx->data, my->data, mx->size );
}

static inline void mmt_mem_reset( mmt_memory_t *mem, size_t size ){
	__check_null( mem, );

	//mem->data points to the memory segment after sizeof( mmt_memory_t )
	mem->data      = mem + 1;
	//safe string
	((char *)mem->data)[ size ] = '\0';
	//store size to head of the memory segment
	mem->size      = size;
	mem->ref_count = 1;
}


#define mmt_free_and_assign_to_null( x ) do{ mmt_mem_free( x ); x = NULL; }while(0)

#endif /* SRC_MMT_ALLOC_H_ */
