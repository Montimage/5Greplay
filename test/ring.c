/*
 * ring.c
 *
 *  Created on: Mar 31, 2017
 *  Created by: Huu Nghia NGUYEN <huunghia.nguyen@me.com>
 */



#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

#include "../src/lib/mmt_lib.h"
#include "../src/lib/lock_free_spsc_ring.h"
#include "../src/lib/system_info.h"


int main( int argc, char **argv){
	lock_free_spsc_ring_t *ring = ring_init( 10 );
	void *data[10] = {NULL};

	int n = ring_push_burst( ring, 6, data );
	ASSERT( n == 0, "Cannot push 6 elements, but only %d", 6-n );

	n = ring_push_burst( ring, 6, data );
	ASSERT( n == 6-2, "Cannot push 2 elements, but %d", 6-n );

	n = ring_pop_burst( ring, 10, data );

	ASSERT( n == 8, "Cannot pop 8 elements, but %d", n );

	ring_free( ring );
	return EXIT_SUCCESS;
}
