/*
 * mmt_bit256.h
 *
 *  Created on: Sep 6, 2018
 *          by: Huu Nghia Nguyen
 *
 * This file implements a set of bit operations on an array of 256 bit.
 */

#ifndef SRC_LIB_MMT_BIT_H_
#define SRC_LIB_MMT_BIT_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct{
	uint8_t data[32]; //we need 32 bytes (256/8) to represent 256 bits
}mmt_bit_t;

static inline mmt_bit_t* mmt_bit_create(){
	 mmt_bit_t *b  = calloc( 1, sizeof( mmt_bit_t) );
	 return b;
}

static inline void mmt_bit_free( mmt_bit_t *b ){
	free( b );
}

/**
 * Set a bit on
 * @param b
 * @param index - index of bit to set
 */
static inline void mmt_bit_set( mmt_bit_t *b, uint8_t index ){
	uint8_t i = index >>  3; // index / 8
	uint8_t j = index  &  7; // index % 8
	b->data[i] |= (1 << j);
}

/**
 * Set a bit off
 * @param b
 * @param index - index of bit to set
 */
static inline void mmt_bit_clear( mmt_bit_t *b, uint8_t index ){
	uint8_t i = index >>  3; // index / 8
	uint8_t j = index  &  7; // index % 8
	b->data[i] &= ~(1 << j);
}

/**
 * Check value of a bit
 * @param b
 * @param index - index of bit to check
 */
static inline bool mmt_bit_check( mmt_bit_t *b, uint8_t index ){
	uint8_t i = index >>  3; // index / 8
	uint8_t j = index  &  7; // index % 8

	return (b->data[i] & (1 << j));
}

#endif /* SRC_LIB_MMT_BIT_H_ */
