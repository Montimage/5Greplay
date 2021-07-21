/*
 * main_plugin_info.c
 *
 *  Created on: Oct 10, 2016
 *  Created by: Huu Nghia NGUYEN <huunghia.nguyen@montimage.com>
 *
 *  Get information of rules encoded in a binary file (.so)
 */
#include "../lib/base.h"
#include "../lib/mmt_log.h"
#include "../lib/mmt_alloc.h"
#include "../engine/plugins_engine.h"
#include <dirent.h>
#include <dlfcn.h>

int info( int argc, char** argv ){
	const rule_info_t *const*rules_arr;
	size_t i, j, n;
	struct tm tm;

	ASSERT( argc <= 2, "Usage: %s [lib_file.so]", argv[0] );

	if( argc == 1)
		//load all plugins from default folder:
		// - /opt/mmt/engine/rules
		// - ./rules
		n = load_mmt_sec_rules( &rules_arr );
	else{
		if( strcmp(argv[1], "-h") == 0 ){
			fprintf( stderr, "Usage: %s [lib_file.so]", argv[0] );
			fprintf( stderr, "\n" );
			return 0;
		}
		//load only one plugin given by argv[1]
		n = load_mmt_sec_rule( &rules_arr, argv[1] );
	}

	//print rules' information
	printf("Found %zu rule%s", n, n<=1? ".": "s." );

	for( i=0; i<n; i++ ){
		printf("\n%zu - Rule id: %d", (i+1), rules_arr[i]->id );
		printf("\n\t- type            : %s",  rules_arr[i]->type_string );
		printf("\n\t- events_count    : %d",  rules_arr[i]->events_count );
		printf("\n\t- variables_count : %d",  rules_arr[i]->proto_atts_count );
		printf("\n\t- variables       : " );
		for( j=0; j<rules_arr[i]->proto_atts_count; j++ )
			printf( "%s%s.%s (%d.%d)",
					j==0? "":", ",
					rules_arr[i]->proto_atts[j].proto, rules_arr[i]->proto_atts[j].att,
					rules_arr[i]->proto_atts[j].proto_id, rules_arr[i]->proto_atts[j].att_id);

		printf("\n\t- description     : %s",  rules_arr[i]->description );
		printf("\n\t- if_satisfied    : %p",  rules_arr[i]->if_satisfied );
		//printf("\n\t- create_instance : %pF",  rules_arr[i]->create_instance );
		//printf("\n\t- hash_message    : %pF",  rules_arr[i]->hash_message );
		tm = *localtime(& rules_arr[i]->version->created_date );
		printf("\n\t- version         : %s (%s - %d-%d-%d %d:%d:%d), dpi version %s",  rules_arr[i]->version->number,
				rules_arr[i]->version->hash,
				tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
				rules_arr[i]->version->dpi );
	}
	printf("\n");
	return 0;
}
