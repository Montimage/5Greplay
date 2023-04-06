/*
 * parse.c
 *
 *  Created on: 26 sept. 2016
 *  Created by: Huu Nghia NGUYEN <huunghia.nguyen@montimage.com>
 */

#include <string.h>
#include "../src/lib/base.h"
#include "../src/lib/expression.h"
#include "../src/lib/mmt_log.h"
#include "../src/lib/mmt_alloc.h"
#include "../src/lib/rule.h"

void print_variable( void *key, void *data, void *user_data, size_t index, size_t total ){
	variable_t *var = (variable_t *)key;
	DEBUG("   - %s.%s", var->proto, var->att );
}

void print_event( void *key, void *data, void *user_data, size_t index, size_t total ){
	char *string;
	rule_event_t *event = (rule_event_t *)data;
	mmt_map_t *map;
	DEBUG(" Event %d", *(uint8_t*)key );
	DEBUG(" + %s", event->description );
	DEBUG(" + number of variables: %zu", get_unique_variables_of_expression( event->expression, &map, NO ));
	expr_stringify_expression( &string, event->expression );
	DEBUG(" + expression: %s", string );
	mmt_map_iterate( map, print_variable, NULL );
	mmt_map_free( map, NO );
	mmt_mem_free( string );
}

int main(){
	rule_t **rule_list;
	size_t rule_count, i;
	mmt_map_t *map;
	rule_count = read_rules_from_file("test/xml/properties_acdc.xml", &rule_list );
	DEBUG( "number of rules: %zu", rule_count );

	for( i=0; i<rule_count; i++ ){
		DEBUG( "====rule id:%d, events count: %zu===", rule_list[i]->id, get_unique_events_of_rule( rule_list[i], & map) );
		mmt_map_iterate(map, print_event, NULL );
		free_a_rule( rule_list[i], YES);
		mmt_map_free( map, NO );
	}

	mmt_free_and_assign_to_null( rule_list );
	mmt_mem_print_info();
	return 0;
}
