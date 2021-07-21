/*
 * mmt_map_t.h
 *
 *  Created on: Nov 2, 2016
 *  Created by: Huu Nghia NGUYEN <huunghia.nguyen@montimage.com>
 *
 * This is an implementation of a generic map on top of a binary-tree.
 * A map is a set of key-value pairs in which each key is unique.
 *	A key and a value can be anything.
 *	One need to provide a function to compare 2 keys, such as strcmp to compare 2 strings.
 *
 */

#ifndef SRC_LIB_MMT_MAP_T_H_
#define SRC_LIB_MMT_MAP_T_H_

#include <inttypes.h>
#include "mmt_array_t.h"

enum compare_result {CMP_LESS = -1, CMP_EQUAL = 0, CMP_GREATER = 1};

static inline int compare_string( const void *a, const void *b ){
	return strcmp( (char *)a, (char *)b );
}
/**
 * Integer comparison
 */
static inline int compare_uint8_t( const void *a, const void *b){
	//ASSERT( a != NULL && b != NULL, "NULL values in compare_uint8_t function %s:%d", __FILE__, __LINE__ );
	return *(uint8_t *)a - *(uint8_t *)b;
}
static inline int compare_uint16_t( const void *a, const void *b){
	//ASSERT( a != NULL && b != NULL, "NULL values in compare_uint16_t function %s:%d", __FILE__, __LINE__ );
	return *(uint16_t *)a - *(uint16_t *)b;
}
static inline int compare_uint32_t( const void *a, const void *b){
	//ASSERT( a != NULL && b != NULL, "NULL values in compare_uint32_t function %s:%d", __FILE__, __LINE__ );
	//return
	if( *(uint32_t *)a  ==  *(uint32_t *)b )
		return 0;
	else if( *(uint32_t *)a  >  *(uint32_t *)b )
		return 1;
	else return -1;
}
static inline int compare_uint64_t( const void *a, const void *b){
	//ASSERT( a != NULL && b != NULL, "NULL values in compare_uint64_t function %s:%d", __FILE__, __LINE__ );
	if( *(uint64_t *)a  ==  *(uint64_t *)b )
		return 0;
	else if( *(uint64_t *)a  >  *(uint64_t *)b )
		return 1;
	else return -1;
}

/**
 * Compare two pointer addresses
 */
static inline int compare_pointer( const void *a, const void *b ){
	if( a == b )
		return 0;
	else if( a > b )
		return 1;
	else
		return -1;
}

/**
 * Binary map structure
 */
typedef void *mmt_map_t;

/**
 * Create and init a binary map
 * - Input:
 * 	+ fun: a function pointer, e.g, strcmp. This function is used to compare keys.
 * 		It takes two parameters being 2 keys to compare.
 * 		It must return:
 * 			- 0 if they are equal
 * 			- -1 if the first key is less than the second
 * 			- 1 if the first key is greater than the second
 * - Output
 * - Return
 * 	+ A pointer points to a binary map
 */
mmt_map_t *mmt_map_init( int (*fun)(const void*, const void*) );
/**
 * Set data to a key
 * - Input:
 * 	+ map: the map to be modified
 * 	+ key : the key of data
 * 	+ data: data to set to the key
 * 	+ override_if_exist: decide to override the old value by the new one
 * - Output
 * - Return:
 * 	+ NULL if no key exists
 * 	+ Pointer points to:
 * 		- the data being overridden if "override_if_exist" = TRUE,
 * 		- otherwise, the parameter "data"
 * - Note:
 * 	+ "key" and "data" should be created by mmt_malloc/mmt_mem_dup. This allows mmt_map_free to free them.
 * 		If not, you must use mmt_map_free( map, NO) to free only map, that does not free keys-data.
 * 	+ when the function return a no-null pointer, to avoid memory leak, one should:
 * 		- free the parameter "key"
 * 		- free the return pointer
 */
void * mmt_map_set_data( mmt_map_t *map, void *key, void *data, bool override_if_exist );
/**
 * Get data of a key
 * - Input:
 * 	+ map:
 * 	+ key
 * - Return:
 * 	+ a pointer points to data having key if exist, otherwise NULL
 */
void *mmt_map_get_data( const mmt_map_t *map, const void *key );


int mmt_map_get_index( const mmt_map_t *map, const void *key );
/**
 * free the map and its keys-data
 */
void mmt_map_free( mmt_map_t *map, bool free_data );

/**
 * Free a map and its keys-data.
 * This differs from #mmt_map_free in the way #key and #data are freed.
 * In function #mmt_map_free, key and data are freed, if #free_data = YES, by #mmt_mem_free.
 * In this function, key and data are freed by the functions given in the parameters.
 */
void mmt_map_free_key_and_data( mmt_map_t *map, void (*free_key_fn)( void *), void (*free_data_fn)( void *)  );

/**
 * Get number of elements in the map
 */
size_t mmt_map_count( const mmt_map_t *map );

/**
 * Iterate a map
 * Input:
 * 	+ map to iterate
 * 	+ an iterate function having the following parameters:
 * 		- key
 * 		- data
 * 		- user_data is the "user_data" parameter
 * 		- index
 * 		- total
 * 	+ user_data
 */
void mmt_map_iterate( const mmt_map_t *map, void (*map_iterate_function)( void *key, void *data, void *user_data, size_t index, size_t total ), void *user_data );

/**
 * Clone a map
 * - Note: this function does not clone #key and #data of each node in the map
 */
mmt_map_t* mmt_map_clone( const mmt_map_t *map );

/**
 * Clone a map by cloning #key and #data of each node of the map
 *
 */
mmt_map_t* mmt_map_clone_key_and_data( const mmt_map_t *map, void* (*clone_key_fn)( void *), void* (*clone_data_fn)( void *)  );

mmt_array_t *mmt_map_convert_to_array( const mmt_map_t *map );

#endif /* SRC_LIB_MMT_MAP_T_H_ */
