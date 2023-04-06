/*
 * set64.c
 *
 *  Created on: Sep 6, 2018
 *          by: Huu Nghia Nguyen
 */


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

#include "../src/lib/mmt_filter_uint.h"

#define LOOP 1000

int main() {
	mmt_set64_t *set = mmt_set64_create();

	mmt_set64_add(set, 257 );
	assert( mmt_set64_check(set, 257) );

	mmt_set64_add(set, 514 );

	assert( !mmt_set64_check(set, 513) );
	assert( !mmt_set64_check(set, 258) );

	int i;
	uint64_t base = RAND_MAX / 2;

	srand( time(NULL) );

	for( i = 0 ; i < LOOP ; i++ ) {
		int val = rand() % base;

		mmt_set64_add(set, val);

		assert( mmt_set64_check(set, val) );
		assert( val != base );

	}

	int ok = 0;
	for( i = 0 ; i < LOOP ; i++ ) {
		int val = rand();
		ok += mmt_set64_check(set, val);
	}
	printf("ok rate: %.3f%%\n", ok*100.0 / LOOP );

	assert(  ! mmt_set64_check(set, base ) );

	return 0;
}
