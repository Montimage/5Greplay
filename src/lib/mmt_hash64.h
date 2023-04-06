/*
 * mmt_hash.h
 *
 *  Created on: May 17, 2017
 *  Created by: Huu Nghia NGUYEN <huunghia.nguyen@me.com>
 */

#ifndef SRC_LIB_MMT_HASH64_H_
#define SRC_LIB_MMT_HASH64_H_

#include "mmt_alloc.h"



static inline uint64_t mmt_hash64_string(const unsigned char *str){
	uint64_t hash = 5381;
	uint8_t c;

	while( (c = *str++) != '\0' )
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

	return hash;
}

/**
 * hash a number to get the best distribution
 * http://xorshift.di.unimi.it/splitmix64.c
 * @param num
 * @return
 */
static inline uint64_t mmt_hash64_number( uint64_t x ){
	x = (x ^ (x >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
	x = (x ^ (x >> 27)) * UINT64_C(0x94d049bb133111eb);
	x = x ^ (x >> 31);
	return x;
}

/**
 * An hash table item
 */
typedef struct mmt_hash64_item_struct{
	uint64_t key;
	void *data;
}mmt_hash64_item_t;

/**
 * MMT Hash table
 */
typedef struct mmt_hash64_struct{
	uint64_t size;
	mmt_hash64_item_t *items;
}mmt_hash64_t;


/**
 * Create a new hash table
 * @param size
 */
static inline mmt_hash64_t* mmt_hash64_create( uint64_t size ){
	size_t i;
	mmt_hash64_t *ret = mmt_mem_alloc( sizeof( mmt_hash64_t ));
	ret->size  = size;
	ret->items = mmt_mem_alloc( sizeof( mmt_hash64_item_t ) * ret->size );
	for( i=0; i<ret->size; i++ ){
		ret->items[i].key  = 0;
		ret->items[i].data = NULL;
	}
	return ret;
}

static inline void mmt_hash64_free( mmt_hash64_t *hash ){
	__check_null(hash, );
	mmt_mem_free( hash->items );
	mmt_mem_free( hash );
}

/**
 * Add a new element to the hash table
 * @param hash
 * @param key
 * @param data
 * @return
 */
static inline bool mmt_hash64_add( mmt_hash64_t *hash, uint64_t key, void *data ){
	uint64_t index   = key % hash->size;
	uint64_t counter = 0;
	//find an available slot
	while( hash->items[ index ].data != NULL ){
		//go to the next slot
		counter ++;
		index ++;

		if( unlikely( counter >= hash->size )){
			log_write( LOG_WARNING, "Cannot insert item to hash table as it is full" );
			return false;
		}

		//return to zero if it goes over
		index %= hash->size;
	}

	hash->items[ index ].key  = key;
	hash->items[ index ].data = data;
	return true;
}

/**
 * Search data by giving a key
 * @param hash
 * @param key
 */
static inline void *mmt_hash64_search( const mmt_hash64_t *hash, uint64_t key ){
	uint64_t index   = key % hash->size;
	uint64_t counter = 0;
	//find an available slot
	while( hash->items[ index ].data != NULL ){
		if( likely( hash->items[ index ].key == key ))
			return hash->items[ index ].data;

		//go to the next slot
		counter ++;
		index ++;

		//find all table but not found
		if( unlikely( counter >= hash->size ))
			return NULL;

		//return to zero if it goes over
		index %= hash->size;
	}

	return NULL;
}

#endif /* SRC_LIB_MMT_HASH64_H_ */
