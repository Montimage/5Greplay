/*
 * main.c
 *
 *  Created on: 16 jul. 2021
 *  Created by: Huu Nghia NGUYEN <huunghia.nguyen@montimage.com>
 *
 *  The main function that will call the other
 */

#include "./lib/mmt_lib.h"

int compile( int argc, char** argv );
int info(    int argc, char** argv );
int replay(  int argc, char** argv );

void _usage( const char *prog ){
	fprintf( stderr, "Usage: %s command [option]", prog );
	fprintf( stderr, "\n - command : is one of the following compile, info, replay");
	fprintf( stderr, "\n - option  : run \"%s command -h\" to get option of each command", prog );
	fprintf( stderr, "\n");
}

int main( int argc, char** argv ){
	if( argc <= 1 ){
		_usage( argv[0] );
		return EXIT_FAILURE;
	}
	const char *prog    = argv[0];
	const char *command = argv[1];
	int ret = EXIT_FAILURE;
	log_open();
	//remove the first element in the parameters
	argc--;
	argv++;
	if( strcmp(command, "compile") == 0 )
		ret = compile( argc, argv );
	else if( strcmp( command, "info" ) == 0 )
		ret = info( argc, argv );
	else if( strcmp( command, "replay" ) == 0 )
		ret = replay( argc, argv );
	else if( strcmp( command, "-h") == 0 ){
		_usage( prog );
		return EXIT_SUCCESS;
	} else{
		fprintf( stderr, "Unknown command \"%s\"\n", command );
		_usage( prog );
		return EXIT_FAILURE;
	}

	log_close();
	return ret;
}
