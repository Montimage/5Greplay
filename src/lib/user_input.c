/*
 * user_input.c
 *
 *  Created on: Nov 18, 2025
 *      Author: nhnghia
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "user_input.h"

#define INPUT_PREFIX "_5GREPLAY_INPUT_"

int user_input_set(const char *name, const char *value ){
	const int overwrite = 1; //overwrite existing variable
	int val;
	char *buf;
	size_t len = strlen(name) + sizeof(INPUT_PREFIX) + 1;
	buf = malloc( len );
	if( !buf )
		return ENOMEM; //Insufficient memory

	snprintf(buf, len, "%s_%s", INPUT_PREFIX, name );

	val = setenv(buf, value, overwrite);
	free( buf );
	return val;
}

const char * user_input_get(const char *name){
	char *buf, *val;
	size_t len = strlen(name) + sizeof(INPUT_PREFIX) + 1;
	buf = malloc( len );
	if( !buf )
		return NULL;

	snprintf(buf, len, "%s_%s", INPUT_PREFIX, name );

	val = getenv(buf);
	free( buf );
	return val;
}
