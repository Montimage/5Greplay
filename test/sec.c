/*
 * sec.c
 *
 *  Created on: Oct 11, 2016
 *  Created by: Huu Nghia NGUYEN <huunghia.nguyen@montimage.com>
 */
#include "../src/lib/mmt_alloc.h"
#include "../src/lib/mmt_log.h"
#include "../src/lib/mmt_single_security.h"

double double_2 = 2;
double double_22 = 22;
double double_18 = 18;

static message_t messages[] = ( message_t[]){
	{.counter = 1, .timestamp = 1452523000158154, .elements_count = 0,
		.elements = (message_element_t[]){
		{.proto_id = 354, .att_id = 6, .data = &double_2}, //tcp - flags
		{.proto_id = 354, .att_id = 2, .data = &double_22},//tcp - dest_port
		{.proto_id = 178, .att_id = 12, .data = "1.1.1.1"},
		{.proto_id = 178, .att_id = 13, .data = "0.0.0.0"}
	} },
	{.counter = 3, .timestamp = 1452524000158154, .elements_count = 0,
			.elements = (message_element_t[]){
		{.proto_id = 354, .att_id = 6, .data = &double_18},
		{.proto_id = 354, .att_id = 1, .data = &double_22},
		{.proto_id = 178, .att_id = 12, .data = "0.0.0.0"},
		{.proto_id = 178, .att_id = 13, .data = "1.1.1.1"}
	} }
};

void callback( const rule_info_t *rule,		//id of rule
		enum verdict_type verdict,
		uint64_t timestamp,  //moment the rule is validated
		uint32_t counter,
		const mmt_map_t *trace,
		void *user_data ){
	char *string = convert_execution_trace_to_json_string( trace );
	DEBUG( "Rule %"PRIu32": %s: %s \n%s\n %s", rule->id, rule->type_string, verdict_type_string[verdict], rule->description, string );
	mmt_mem_free( string );
}

int main( int argc, char **argv ){
	const rule_info_t **rules_array;
	size_t size, i,j;
	mmt_single_sec_handler_t *handler;
	size = mmt_sec_get_rules_info( &rules_array );
	handler = mmt_sec_register( rules_array, size, callback, NULL );

	size = sizeof( messages ) / sizeof( message_t );
	DEBUG( "Testing %zu messages ... ", size );

	//for each message
	for( i=0; i<size; i++ ){
		messages[i].elements_count = 4; //sizeof( elements[i] ) / sizeof( message_element_t* );
		mmt_sec_process( handler, &messages[i] );
	}

	mmt_sec_unregister( handler );
	mmt_mem_free( rules_array );
	return 0;
}
