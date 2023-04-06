/*
 * mmt_hash.h
 *
 *  Created on: May 18, 2017
 *  Created by: Huu Nghia NGUYEN <huunghia.nguyen@me.com>
 *
 * This implements a hash table.
 * Each element of the hash table is a pair <key, value> which can be any data type.
 *
 */

#ifndef SRC_LIB_MMT_HASH_H_
#define SRC_LIB_MMT_HASH_H_

#include "mmt_alloc.h"
#include "mmt_log.h"

#define CHILDREN_COUNT 256

typedef struct mmt_hash_struct{
	void *data;
	struct mmt_hash_struct* children[ CHILDREN_COUNT ];
} mmt_hash_t;


/**
 * Create a hash table
 * @return
 */
static inline mmt_hash_t *mmt_hash_create(){
	int i;
	mmt_hash_t *ret = mmt_mem_alloc( sizeof( mmt_hash_t) );
	ret->data = NULL;
	for( i=0; i<CHILDREN_COUNT; i++ )
		ret->children[i] = NULL;
	return ret;
}

/**
 * Add an element to the hash table.
 * It can override the existing element having the same key if #is_override = true.
 *
 * @param table
 * @param key
 * @param key_size
 * @param data
 * @param is_override
 * @return the old value of the element having the same key.
 * 	This value is NULL if does not exist the such of key in the hash table
 */
static inline void *mmt_hash_add( mmt_hash_t *table,
		const void *key, uint8_t key_size, void *data,
		bool is_override){
	const uint8_t *c = (uint8_t *) key;
	mmt_hash_t *item = table, *tmp;
	void *old_data;

	//for each byte in *key
	while( key_size > 0 ){
		//does not exist a child
		if( item->children[ *c ] == NULL ){
			tmp = mmt_hash_create();
			item->children[ *c ] = tmp;
		}

		//go inside the child
		item = item->children[ *c ];

		//goto the next byte in the key
		c++;
		key_size --;
	}

	old_data = item->data;
	//not allow to override the
	if( is_override || item->data == NULL )
		item->data = data;

	return old_data;
}

/**
 * Search in the hash table an element having the #key.
 * @param table
 * @param key
 * @param key_size is number of bytes of #key
 * @return element's data
 */
static inline void* mmt_hash_search( const mmt_hash_t *table, const void *key, uint8_t key_size ){
	const uint8_t *c = (uint8_t *) key;
	const mmt_hash_t *item = table;

	//for each byte in *key
	while( key_size > 0 ){
		item = item->children[ *c ];
		//does not exist
		if( item == NULL )
			return NULL;
		//goto the next byte in the key
		c++;
		key_size --;
	}

	return item->data;
}

/**
 * Free the hash table
 * @param table
 */
static inline void mmt_hash_free( mmt_hash_t *table ){
	int i;
	if( table == NULL )
		return;

	//free the children
	for( i=0; i<CHILDREN_COUNT; i++ )
		mmt_hash_free( table->children[i] );

	//free the table
	mmt_mem_free( table );
}

#endif /* SRC_LIB_MMT_HASH_H_ */
