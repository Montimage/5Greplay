/*
 * main_gen_plugin.c
 *
 *  Created on: 26 sept. 2016
 *  Created by: Huu Nghia NGUYEN <huunghia.nguyen@montimage.com>
 *
 *  Parse rules in .xml file, then generate .c file, then compile to a plugin .so file.
 */

#include <mmt_core.h>
#include "../lib/mmt_lib.h"
#include "../engine/expression.h"
#include "../engine/rule.h"
#include "../engine/gen_code.h"

#define MAX_STRING_LEN 10000

int compile( int argc, char** argv ){
	rule_t **rule_list;
	size_t rule_count, i;
	char c_file[MAX_STRING_LEN];
	char gcc_param[MAX_STRING_LEN];

	const char* xml_file, *output_file;
	char *embedded_functions;
	int ret;

	if( argc != 3 && argc != 4){
		fprintf( stderr, "Usage: %s output_file property_file option", argv[0] );
		fprintf( stderr, "\n - output_file   : path of file containing result that can be either a .c file or .so file");
		fprintf( stderr, "\n - property_file : path to property file to read");
		fprintf( stderr, "\n - option        : ");
		fprintf( stderr, "\n      + \"-c\"     : generate only code c" );
		fprintf( stderr, "\n      + otherwise: generate code c, then compile to .so file.");
		fprintf( stderr, "\n                   This option will be transfered to gcc, for example, \"-I /tmp/include -lmath\"");
		fprintf( stderr, "\n");
		return 1;
	}
	xml_file     = argv[2];
	output_file  = argv[1];

	//check output file
	if( argc == 4 && strcmp( argv[3], "-c") == 0 ){
		if( !str_end_with( output_file, ".c" ) ){
			log_write( LOG_ERR, "output_file must be end with .c");
			return 1;
		}

		snprintf(c_file, sizeof( c_file), "%s", output_file );
	}else{
		if( !str_end_with( output_file, ".so" ) ){
			log_write( LOG_ERR, "output_file must be end with .so");
			return 1;
		}
		snprintf(c_file, sizeof( c_file), "%s.c", output_file );
	}

	//init mmt_dpi extraction to load list of avaiable protocols
	init_extraction();

	//read rule from .xml file
	rule_count = read_rules_from_file( xml_file, &rule_list, &embedded_functions );

	//generate rules to .c code
	generate_fsm( c_file, rule_list, rule_count, embedded_functions );

	if( argc == 4 && strcmp( argv[3], "-c") == 0 ){
		log_write( LOG_INFO, "Encoded %zu rules from \"%s\" to \"%s\"", rule_count, xml_file, c_file );
		log_write( LOG_INFO, "To compile, use: /usr/bin/gcc -fPIC -shared %s -o output.so", c_file );
	}else{
		//compile code file
		if( argc == 3 )
			ret = compile_gen_code(output_file, c_file,"./src/lib -I./src/engine -I ./src/dpi -I/opt/mmt/dpi/include" );
		else{
			snprintf( gcc_param, sizeof( gcc_param), "./src/lib -I./src/engine -I ./src/dpi -I/opt/mmt/dpi/include %s", argv[3] );
			ret = compile_gen_code(output_file, c_file, gcc_param );
		}

		//DEBUG( "ret = %d", ret );
		if( ret == 0 ){
			log_write( LOG_INFO, "Encoded %zu rules from \"%s\" to \"%s\"", rule_count, xml_file, output_file );

#ifndef DEBUG_MODE
			//delete .c file
			remove( c_file );
#endif
		}else
			log_write( LOG_ERR, "Cannot encode rule \"%s\". Check options.", xml_file );
	}

	//free each rule
	for( i=0; i<rule_count; i++ )
		free_a_rule( rule_list[i], true);

	mmt_mem_free( rule_list );
	mmt_mem_free( embedded_functions );
	close_extraction();// close mmt_dpi
	return 0;
}
