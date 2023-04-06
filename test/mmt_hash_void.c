/*
 * mmt_hash_void.c
 *
 *  Created on: May 18, 2017
 *  Created by: Huu Nghia NGUYEN <huunghia.nguyen@me.com>
 *
 * Build from ../src using command: make test.mmt_hash_void && ./test.mmt_hash_void
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include "../src/lib/mmt_lib.h"

int main( int argc, char **argv){
	mmt_hash_t *hash_table = mmt_hash_create();
	ASSERT( mmt_hash_add(hash_table, "hi", 2, "_hi", false) == NULL, "Override the old value" );
	ASSERT( mmt_hash_add(hash_table, "he", 2, "_he", false) == NULL, "Override the old value" );
	ASSERT( mmt_hash_add(hash_table, "xxxxxxxxxx", 10, "_xxxxxxxxx", false) == NULL, "Override the old value" );

	log_write( LOG_INFO, "%s", (char *)mmt_hash_search( hash_table, "hi", 2));
	log_write( LOG_INFO, "%s", (char *)mmt_hash_search( hash_table, "he", 2));
	log_write( LOG_INFO, "%s", (char *)mmt_hash_search( hash_table, "xxxxxxxxxx", 10));

	mmt_hash_free( hash_table );
	return EXIT_SUCCESS;
}
