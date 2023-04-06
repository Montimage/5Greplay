/*
 * pointer.c
 *
 *  Created on: 23 sept. 2016
 *  Created by: Huu Nghia NGUYEN <huunghia.nguyen@montimage.com>
 */

#include "../src/lib/base.h"
#include "../src/lib/expression.h"
#include "../src/lib/mmt_log.h"
#include "../src/lib/mmt_alloc.h"

void set_string( char **p ){
	*p = mmt_mem_alloc(3);
	memcpy( *p, "xxx", 3);
}

void set_null( int *p){
	p = NULL;
}

void free_pointer( void * p ){
	free( p );
}

void set_num( uint64_t *p){
	*p = 10;
}

int main(){
	char *ptr = NULL;
	int *p = mmt_mem_alloc( sizeof( int ));
	*p = 5;
	set_null( p );
	ASSERT( p != NULL, "Not null %d", *p);

	set_string( &ptr );
	DEBUG( "%s", ptr );

	mmt_mem_free( ptr );
	ptr = NULL;

	set_string( &ptr );
	DEBUG( "%s", ptr );


	mmt_mem_free( ptr );
	ptr = NULL;

	set_num( p );
	DEBUG( "%d", *p );

	//free_pointer( p );

	return 0;
}
