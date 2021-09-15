/*
 * main.c
 *
 *  Created on: 16 jul. 2021
 *  Created by: Huu Nghia NGUYEN <huunghia.nguyen@montimage.com>
 *
 *  The main function that will call the other
 */

#include "./lib/mmt_lib.h"
#include <mmt_core.h>

int compile( int argc, char** argv );
int info(    int argc, char** argv );
int replay(  int argc, char** argv );
int list(    int argc, char** argv );
int extract( int argc, char ** argv);

char *str_hex2str(char *hstr, int start_index, int end_index);

void _usage( const char *prog ){
	char *s = str_hex2str(NULL, 0, 0);
	fprintf( stderr, "Usage: %s command [option]", prog );
	fprintf( stderr, "\n - command : is one of the following: compile, info, extract, replay");
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

	//print version of DPI
	log_write( LOG_INFO, "5Greplay v%s-%s using DPI v%s is running on pid %d",
			VERSION_NUMBER, GIT_VERSION, /*These 2 parameters are given by Makefile*/
			mmt_version(),
			getpid() );

	//remove the first element in the parameters
	argc--;
	argv++;

	if( strcmp(command, "compile") == 0 )
		ret = compile( argc, argv );
	else if( strcmp( command, "info" ) == 0 )
		ret = info( argc, argv );
	else if( strcmp( command, "replay" ) == 0 )
		ret = replay( argc, argv );
	else if( strcmp( command, "list" ) == 0 )
		ret = list( argc, argv );
	else if( strcmp( command, "extract" ) == 0 )
		ret = extract( argc, argv );
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
