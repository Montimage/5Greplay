/*
 * message_t.c
 *
 *  Created on: Apr 3, 2017
 *  Created by: Huu Nghia NGUYEN <huunghia.nguyen@me.com>
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include "../src/rule/message_t.h"

void print( const message_t *msg ){
	int i;
	for( i=0; i<msg->_elements_index; i++ )
		printf( "%c%s", i==0? '\n':' ', (char *)msg->elements[i].data );
}

int main( int argc, char **argv){
	message_t *msg = create_message_t( 5 );
	int i;

	set_element_data_message_t( msg, 1, 2, "a", MMT_SEC_MSG_DATA_TYPE_STRING, 1 );

	print( msg );

	set_element_data_message_t( msg, 2, 7, "c", MMT_SEC_MSG_DATA_TYPE_STRING, 1 );

	print( msg );

	set_element_data_message_t( msg, 5, 9, "d", MMT_SEC_MSG_DATA_TYPE_STRING, 1 );

	print( msg );

	set_element_data_message_t( msg, 1, 7, "b", MMT_SEC_MSG_DATA_TYPE_STRING, 1 );

	print( msg );

	set_element_data_message_t( msg, 2, 7, "X", MMT_SEC_MSG_DATA_TYPE_STRING, 1 );

	print( msg );

	return EXIT_SUCCESS;
}
