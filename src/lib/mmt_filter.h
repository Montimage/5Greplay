/*
 * mmt_filter64.h
 *
 *  Created on: Sep 6, 2018
 *          by: Huu Nghia Nguyen
 *
 * This file implements a filter to check a number
 * The filter supports basically 3 operations:
 * (1) add a number.
 * (2) remove a number.
 * (3) check whether a number was added to the filter
 */

#ifndef SRC_LIB_MMT_FILTER_H_
#define SRC_LIB_MMT_FILTER_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "mmt_bit.h"

/*********************************************************************************/
/**                       A filter of 32 bit numbers                               **/
/*********************************************************************************/

typedef struct{
	mmt_bit_t data[4]; //we need 4 blocks to represent 4 bytes of 32 bit numbers
}mmt_filter32_t;


static inline mmt_filter32_t* mmt_filter32_create(){
	mmt_filter32_t *ret = calloc(1, sizeof( mmt_filter32_t ));
	return ret;
}

static inline void mmt_filter32_free( mmt_filter32_t *filter ){
	free( filter );
}

/**
 * Add a number to the filter
 * @param filter
 * @param val
 */
static inline void mmt_filter32_add( mmt_filter32_t *filter, uint32_t val ){
	uint8_t *p = (uint8_t *) &val;
	mmt_bit_set( &filter->data[0], p[0] );
	mmt_bit_set( &filter->data[1], p[1] );
	mmt_bit_set( &filter->data[2], p[2] );
	mmt_bit_set( &filter->data[3], p[3] );
}

/**
 * Check whether a number was added to the the filter.
 *
 * @param filter
 * @param val
 * @return
 * - false if the number has not added
 * - true, otherwise. Note that, it does not mean that the number was added to the filter.
 */
static inline bool mmt_filter32_check( mmt_filter32_t *filter, uint32_t val ){
	uint8_t *p = (uint8_t *) &val;
	bool ret = (
	   mmt_bit_check( &filter->data[0], p[0] )
	&& mmt_bit_check( &filter->data[1], p[1] )
	&& mmt_bit_check( &filter->data[2], p[2] )
	&& mmt_bit_check( &filter->data[3], p[3] )
	);
	return ret;
}

/**
 * Remove a number from the filter.
 * The function is always success even if the number  does not exist.
 * @param filter
 * @param val
 */
static inline void mmt_filter32_rm( mmt_filter32_t *filter, uint32_t val ){
	uint8_t *p = (uint8_t *) &val;
	mmt_bit_clear( &filter->data[0], p[0] );
	mmt_bit_clear( &filter->data[1], p[1] );
	mmt_bit_clear( &filter->data[2], p[2] );
	mmt_bit_clear( &filter->data[3], p[3] );
}



/*********************************************************************************/
/**                       A filter of 64 bit numbers                               **/
/*********************************************************************************/

typedef struct{
	mmt_filter32_t data[2]; //we need 2 block of 4bytes to represent numbers of 64bit
}mmt_filter64_t;


static inline mmt_filter64_t* mmt_filter64_create(){
	mmt_filter64_t *ret = calloc(1, sizeof( mmt_filter64_t ));
	return ret;
}

static inline void mmt_filter64_free( mmt_filter64_t *filter ){
	free( filter );
}

/**
 * Add a number to the filter
 * @param filter
 * @param val
 */
static inline void mmt_filter64_add( mmt_filter64_t *filter, uint64_t val ){
	uint32_t *p = (uint32_t *) &val;
	mmt_filter32_add( &filter->data[0], p[0] );
	mmt_filter32_add( &filter->data[1], p[1] );
}

/**
 * Check whether the filter contains a number
 * @param filter
 * @param val
 * @return
 * - false if the number has not added
 * - true, otherwise. Note that, it does not mean that the number was added to the filter.
 */
static inline bool mmt_filter64_check( mmt_filter64_t *filter, uint64_t val ){
	uint32_t *p = (uint32_t *) &val;
	bool ret = (
	   mmt_filter32_check( &filter->data[0], p[0] )
	&& mmt_filter32_check( &filter->data[1], p[1] )
	);
	return ret;
}

/**
 * Remove a number from the filter.
 * The function is always success even if the number  does not exist.
 * @param filter
 * @param val
 */
static inline void mmt_filter64_rm( mmt_filter64_t *filter, uint64_t val ){
	uint32_t *p = (uint32_t *) &val;
	mmt_filter32_rm( &filter->data[0], p[0] );
	mmt_filter32_rm( &filter->data[1], p[1] );
}


#endif /* SRC_LIB_MMT_FILTER_H_ */
