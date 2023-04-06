/*
 * verdict_printer.c
 *
 *  Created on: Dec 19, 2016
 *  Created by: Huu Nghia NGUYEN <huunghia.nguyen@montimage.com>
 */

#include <stdlib.h>
#include <string.h>
#include "verdict_printer.h"

#include "../lib/mmt_lib.h"

#define MAX_STRING_LEN 500

void init_file();
void send_message_to_file(const char*);
void close_file();

enum output_mode{
	OUTPUT_NONE  = 0,
	OUTPUT_FILE  = 1, /**< Output to file */
};

static enum output_mode output_mode = OUTPUT_NONE;

void verdict_printer_init( const char *file_string, int interval ){
	//output to file
	if( file_string != NULL && file_string[0] != '\0' ){
		init_file( file_string, interval );

		output_mode |= OUTPUT_FILE;
	}
}

void verdict_printer_send( const char* msg ){
	//and bit
	if( output_mode & OUTPUT_FILE )
		send_message_to_file( msg );

}


void verdict_printer_free( ){
	if( output_mode & OUTPUT_FILE )
		close_file();
}
