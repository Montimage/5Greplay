/*
 * rule.c
 *
 *  Created on: 20 sept. 2016
 *  Created by: Huu Nghia NGUYEN <huunghia.nguyen@montimage.com>
 */

#include <stdio.h>
#include <errno.h>
#include <libxml/xmlreader.h>

#include "rule.h"


#define str_equal(X,Y) xmlStrcmp( (const xmlChar*)X, (const xmlChar*)Y) == 0

enum time_unit {YEAR, MONTH, DAY, HOUR, MINUTE, SECOND, MILI_SECOND, MICRO_SECOND};

void rule_free_an_event( rule_event_t *event, bool free_data){
	if( event == NULL ) return;
	if( free_data ){
		mmt_free_and_assign_to_null( event->description );
		expr_free_an_expression( event->expression, free_data );
	}
	mmt_free_and_assign_to_null( event );
}

//pre-define
void _free_rule_node( rule_node_t *node );
void rule_free_an_operator( rule_operator_t *operator, bool free_data){

	if( operator == NULL ) return;
	if( free_data ){
		mmt_free_and_assign_to_null( operator->description );
		mmt_free_and_assign_to_null( operator->delay );
		_free_rule_node( operator->context );
		_free_rule_node( operator->trigger );
	}
	mmt_free_and_assign_to_null( operator );
}

void _free_rule_node( rule_node_t *node ){
	if( node == NULL ) return;

	if( node->type == RULE_NODE_TYPE_EVENT )
		rule_free_an_event( node->event, YES );
	else if( node->type == RULE_NODE_TYPE_OPERATOR )
		rule_free_an_operator( node->operator, YES );
	else{
		log_write( LOG_WARNING,"Unexpected rule_node_type: %d", node->type );
	}
	mmt_free_and_assign_to_null( node );
}

void free_a_rule( rule_t *rule, bool free_data){
	if( rule == NULL )
		return;
	if( free_data ){
		mmt_free_and_assign_to_null( rule->description );
		mmt_free_and_assign_to_null( rule->delay );
		mmt_free_and_assign_to_null( rule->if_satisfied);
		mmt_free_and_assign_to_null( rule->if_not_satisfied );
		mmt_free_and_assign_to_null( rule->keep_state );
		_free_rule_node( rule->context );
		_free_rule_node( rule->trigger );
	}
	mmt_free_and_assign_to_null( rule );
}

static inline void _update_delay_to_micro_second( rule_delay_t *delay, int delay_time_unit ){
	switch( delay_time_unit ){
	case YEAR:
		delay->time_max *= 12;
		delay->time_min *= 12;
		/* No break are required in the "cases" */
	case MONTH:
		delay->time_max *= 30;
		delay->time_min *= 30;
	case DAY:
		delay->time_max *= 24;
		delay->time_min *= 24;
	case HOUR:
		delay->time_max *= 60;
		delay->time_min *= 60;
	case MINUTE:
		delay->time_max *= 60;
		delay->time_min *= 60;
	case SECOND:
		delay->time_max *= 1000;
		delay->time_min *= 1000;
	case MILI_SECOND:
		delay->time_max *= 1000;
		delay->time_min *= 1000;
	case MICRO_SECOND:
		return;
	}
}

static inline int _get_sign( const char *str ){
	switch( str[ strlen(str) - 1 ]){
	case '-':  //for delay_max, sign = -1 means less than
		return -1;
	case '+':
		return 1;  //for delay_min, sign = +1 means greater than
	default:
		return 0;
	}
}

static inline uint64_t _parse_unsigned_int( const char * str, const char *error_message ){
	ASSERT( str[0] != '-', "%s: %s", error_message, str );
	return atoll( str );
}

/**
 * Create and init values for a delay struct
 */
rule_delay_t *_parse_rule_delay( const xmlNode *xml_node ){
	const xmlAttr *xml_attr;
	const xmlChar *xml_attr_name;
	xmlChar *ptr;
	char *xml_attr_value;
	int delay_time_unit   = SECOND;
	int time_min_sign, time_max_sign, counter_min_sign, counter_max_sign;
	bool has_delay = NO;
	rule_delay_t *delay = mmt_mem_alloc( sizeof( rule_delay_t ));

	delay->time_min         = delay->time_max         = 0;
	delay->counter_min      = delay->counter_max      = 0;

	time_min_sign    = time_max_sign    = 0;
	counter_min_sign = counter_max_sign = 0;


	//parse attributes of the node
	xml_attr = xml_node->properties;
	while( xml_attr != NULL && xml_attr->name != NULL){
		xml_attr_name  = xml_attr->name;
		ptr            = xmlGetProp( (xmlNodePtr) xml_node, xml_attr_name );
		xml_attr_value = (char *)ptr;

		if( str_equal( xml_attr_name, "delay_min" ) ){
			ASSERT( xml_attr_value[0] != '-', "Error 13j: \"delay_min\" must not be negative. Note: you can use value=BEFORE" );

			delay->time_min = (uint64_t) atoll( xml_attr_value );
			time_min_sign = _get_sign( xml_attr_value );
			has_delay = YES;
		}else if( str_equal( xml_attr_name, "delay_max" ) ){
			ASSERT( xml_attr_value[0] != '-', "Error 13j: \"delay_max\" must not be negative. Note: you can use value=BEFORE" );
			delay->time_max = (uint64_t) atoll( xml_attr_value );
			time_max_sign = _get_sign( xml_attr_value );
			has_delay = YES;
		}else if( str_equal( xml_attr_name, "counter_min" ) ){
			ASSERT( xml_attr_value[0] != '-', "Error 13j: \"counter_min\" must not be negative. Note: you can use value=BEFORE" );
			delay->counter_min = (uint64_t) atoll( xml_attr_value );
			counter_min_sign = _get_sign( xml_attr_value );
			has_delay = YES;
		}else if( str_equal( xml_attr_name, "counter_max" ) ){
			ASSERT( xml_attr_value[0] != '-', "Error 13j: \"counter_max\" must not be negative. Note: you can use value=BEFORE" );
			delay->counter_max= (uint64_t) atoll( xml_attr_value );
			counter_max_sign = _get_sign( xml_attr_value );
			has_delay = YES;
		}else if( str_equal( xml_attr_name, "delay_units" ) ){
			if( str_equal( xml_attr_value, "Y"))
				delay_time_unit = YEAR;
			else if( str_equal( xml_attr_value, "M"))
				delay_time_unit = MONTH;
			else if( str_equal( xml_attr_value, "D"))
				delay_time_unit = DAY;
			else if( str_equal( xml_attr_value, "h"))
				delay_time_unit = HOUR;
			else if( str_equal( xml_attr_value, "m"))
				delay_time_unit = MINUTE;
			else if( str_equal( xml_attr_value, "s"))
				delay_time_unit = SECOND;
			else if( str_equal( xml_attr_value, "ms"))
				delay_time_unit = MILI_SECOND;
			else if( str_equal( xml_attr_value, "mms"))
				delay_time_unit = MICRO_SECOND;
			else{
				ABORT("Error 13d: Unexpected time_units: %s", xml_attr_value );
			}
			has_delay = YES;
		}
		xmlFree( ptr );
		xml_attr = xml_attr->next;
	}

	if( has_delay ){
		ASSERT( (delay->time_min <= delay->time_max),
				"Error 13da: delay_min must not be greater than delay_max");
		ASSERT( (delay->counter_min <= delay->counter_max),
				"Error 13db: counter_min must not be greater than counter_max");

		_update_delay_to_micro_second( delay, delay_time_unit );

		delay->time_min += time_min_sign;
		delay->time_max += time_max_sign;
		delay->counter_min  += counter_min_sign;
		delay->counter_max  += counter_max_sign;

		//ASSERT( !(delay->time_min == delay->time_max && delay->time_max == 0), "Delay must not be zero!");
	}
	return delay;
}

static rule_event_t *_parse_an_event(const xmlNode *xml_node ){
	rule_event_t event, *ret = NULL;
	xmlAttr *xml_attr;
	const xmlChar *xml_attr_name;
	xmlChar *xml_attr_value;

	//init default values
	event.id          = UNKNOWN;
	event.description = NULL;
	event.expression  = NULL;

	//parse attributes of the node
	xml_attr = xml_node->properties;
	while( xml_attr != NULL && xml_attr->name != NULL){
		xml_attr_name  = xml_attr->name;
		xml_attr_value = xmlGetProp( (xmlNodePtr) xml_node, xml_attr_name );

		//for each attribute
		if( str_equal( xml_attr_name, "boolean_expression" ) )
			parse_expression( &event.expression, (const char *) xml_attr_value, strlen( (const char*) xml_attr_value ) );
		else if( str_equal( xml_attr_name, "description" ) )
			event.description = mmt_mem_dup( xml_attr_value, strlen( (const char*) xml_attr_value ));
		else if( str_equal( xml_attr_name, "event_id" ) )
			event.id = _parse_unsigned_int( (const char*) xml_attr_value, "Error 14a: event_id must not be negative" );
		else if( str_equal( xml_attr_name, "value" ) ){
			//do nothing
		}else
			log_write( LOG_WARNING, "Warning 13e: Unexpected attribute %s in tag event", xml_attr_name );

		xmlFree( xml_attr_value );
		xml_attr = xml_attr->next;
	}

	ret = mmt_mem_dup( &event, sizeof( rule_event_t ));
	return ret;
}

//pre-define this function
static rule_node_t *_parse_a_rule_node( const xmlNode *xml_node );

static inline int _get_value( const xmlChar *xml_attr_value ){
	if( str_equal( xml_attr_value, "THEN" ) )
		return RULE_VALUE_THEN;
	if( str_equal( xml_attr_value, "BEFORE" ) )
		return RULE_VALUE_BEFORE;
	else if( str_equal( xml_attr_value, "OR" ) )
		return RULE_VALUE_OR;
	else if( str_equal( xml_attr_value, "AND" ) )
		return RULE_VALUE_AND;
	else if( str_equal( xml_attr_value, "NOT" ) )
		return RULE_VALUE_NOT;
	else if( str_equal( xml_attr_value, "COMPUTE" ) )
		return RULE_VALUE_COMPUTE;
	else
		ABORT( "Error 13d: Unexpected attribute value=\"%s\"", xml_attr_value );
	return UNKNOWN;
}

static rule_operator_t *_parse_an_operator( const xmlNode *xml_node ){
	rule_operator_t operator, *ret = NULL;
	xmlAttr *xml_attr;
	const xmlChar *xml_attr_name;
	xmlChar *xml_attr_value;

	//init default values
	operator.value        = RULE_VALUE_COMPUTE;
	operator.description  = NULL;
	operator.repeat_times = 1;
	operator.context      = NULL;
	operator.trigger      = NULL;
	operator.delay        = _parse_rule_delay( xml_node );

	//parse attributes of the node
	xml_attr = xml_node->properties;
	while( xml_attr != NULL && xml_attr->name != NULL){
		xml_attr_name  = xml_attr->name;
		xml_attr_value = xmlGetProp( (xmlNodePtr) xml_node, xml_attr_name );

		//for each attribute
		if( str_equal( xml_attr_name, "value" ) ){
			operator.value = _get_value( xml_attr_value );
		}else if( str_equal( xml_attr_name, "description" ) )
			operator.description = mmt_mem_dup( xml_attr_value, strlen( (const char*) xml_attr_value ));
		/*
			else
				log_write( LOG_WARNING, "Warning 13e: Unexpected attribute %s in tag operator", xml_attr_name );
		 */
		xmlFree( xml_attr_value );
		xml_attr = xml_attr->next;
	}
	//go inside the node
	xml_node = xml_node->children;
	while( xml_node ){
		if( xml_node->type == XML_ELEMENT_NODE ){
			if( operator.context == NULL )
				operator.context = _parse_a_rule_node( xml_node );
			else if( operator.trigger == NULL )
				operator.trigger = _parse_a_rule_node( xml_node );
			else
				ABORT("Error 13f: Unexpected more than 2 children in property tag");
		}

		xml_node = xml_node->next;
	}

	//when rule type = "COMPUTE", no need the second <event>
	if (operator.value == RULE_VALUE_COMPUTE ){
		ASSERT( (operator.trigger == NULL),
			"Error 13g: Do not expect trigger for operator COMPUTE. Only one <event> is required.");
		ASSERT( (operator.delay->counter_max == 0 && operator.delay->time_max == 0
				&& operator.delay->counter_max == 0 && operator.delay->counter_min == 0 ),
			"Error 13g: Do not expect delay for operator COMPUTE");
	}
	else {
		//when rule type != COMPUTE, we need 2 events: context and trigger
		ASSERT( (operator.context != NULL && operator.trigger != NULL ),
			"Error 13g: Require 2 <event> for context and trigger for operator " );
	}

	ret = mmt_mem_dup( &operator, sizeof( rule_operator_t ));
	return ret;
}


static rule_node_t *_parse_a_rule_node( const xmlNode *xml_node ){
	rule_node_t *rule_node = mmt_mem_alloc( sizeof( rule_node_t ));
	//init default value
	rule_node->type     = UNKNOWN;
	rule_node->operator = NULL;
	rule_node->event    = NULL;

	if( str_equal( xml_node->name, "operator" ) ){
		rule_node->type     = RULE_NODE_TYPE_OPERATOR;
		rule_node->operator = _parse_an_operator( xml_node );
	}else if( str_equal( xml_node->name, "event" ) ){
		rule_node->type  = RULE_NODE_TYPE_EVENT;
		rule_node->event = _parse_an_event( xml_node );
	}
	else{
		log_write( LOG_WARNING, "Warning 13g: Unexpected tag %s", xml_node->name );
		mmt_free_and_assign_to_null( rule_node );
		rule_node = NULL;
	}
	return rule_node;
}

static rule_t *_parse_a_rule( const xmlNode *xml_node ){
	rule_t rule, *ret = NULL;
	xmlAttr *xml_attr;
	const xmlChar *xml_attr_name;
	xmlChar *xml_attr_value;

	//init default values
	rule.id               = UNKNOWN;
	rule.value            = RULE_VALUE_COMPUTE;
	rule.type             = RULE_TYPE_FORWARD;
	rule.description      = NULL;
	rule.if_satisfied     = NULL;
	rule.if_not_satisfied = NULL;
	rule.keep_state       = NULL;
	rule.context          = NULL;
	rule.trigger          = NULL;
	rule.delay            = _parse_rule_delay( xml_node );

	//parse attributes of the node
	xml_attr = xml_node->properties;
	while( xml_attr != NULL && xml_attr->name != NULL){
		xml_attr_name  = xml_attr->name;
		xml_attr_value = xmlGetProp( (xmlNodePtr) xml_node, xml_attr_name );

		//for each attribute
		if( str_equal( xml_attr_name, "property_id" ) )
			rule.id = (uint16_t) _parse_unsigned_int( (const char*) xml_attr_value, "Error 14b: property_id must not be negative" );
		else if( str_equal( xml_attr_name, "value" ) )
			rule.value = _get_value( xml_attr_value );
		else if( str_equal( xml_attr_name, "type_property" ) ){
			if( str_equal( xml_attr_value, "ATTACK" ) )
				rule.type = RULE_TYPE_ATTACK;
			else if( str_equal( xml_attr_value, "SECURITY" ) )
				rule.type = RULE_TYPE_SECURITY;
			else if( str_equal( xml_attr_value, "EVASION" ) )
				rule.type = RULE_TYPE_EVASION;
			else if( str_equal( xml_attr_value, "TEST" ) )
				rule.type = RULE_TYPE_TEST;
			//mmt-5greplay
			else if( str_equal( xml_attr_value, "FORWARD" ) )
				rule.type = RULE_TYPE_FORWARD;
			else
				ABORT( "Error 13c: Unexpected type_property: %s. Expect one of the following: ATTACK, EVASION, TEST, FORWARD", xml_attr_value );
		}else if( str_equal( xml_attr_name, "description" ) )
			rule.description = mmt_mem_dup( xml_attr_value, strlen( (const char*) xml_attr_value ));
		else if( str_equal( xml_attr_name, "if_satisfied" ) )
			rule.if_satisfied = mmt_mem_dup( xml_attr_value, strlen( (const char*) xml_attr_value ));
		else if( str_equal( xml_attr_name, "if_not_satisfied" ) )
			rule.if_not_satisfied = mmt_mem_dup( xml_attr_value, strlen( (const char*) xml_attr_value ));
		else if( str_equal( xml_attr_name, "keep_state" ) )
			rule.keep_state = mmt_mem_dup( xml_attr_value, strlen( (const char*) xml_attr_value ));
		/*
		else
			mmt_log(WARN, "Warning 13e: Unexpected attribute %s in tag property", xml_attr_name );
		*/
		xmlFree( xml_attr_value );
		xml_attr = xml_attr->next;
	}

	//go inside the node
	xml_node = xml_node->children;
	while( xml_node ){
		if( xml_node->type == XML_ELEMENT_NODE ){
			if( rule.context == NULL )
				rule.context = _parse_a_rule_node( xml_node );
			else if( rule.trigger == NULL )
				rule.trigger = _parse_a_rule_node( xml_node );
			else{
				ABORT("Error 13f: Unexpected more than 2 children in property tag");
			}
		}

		xml_node = xml_node->next;
	}

	//when rule type = "COMPUTE", no need the second <event>
	if( rule.value == RULE_VALUE_COMPUTE ){
		ASSERT( (rule.trigger == NULL ),
			"Error 13g: Do not expect trigger for rule %d. Only one <event> is required.", rule.id );
		ASSERT( (rule.delay->counter_max == 0 && rule.delay->time_max == 0),
			"Error 13g: Do not expect delay for operator COMPUTE for rule %d", rule.id );


		//change COMPUTE X rule to (X THEN true)
		rule.value = RULE_VALUE_THEN;

		rule_event_t *event = mmt_mem_alloc( sizeof( rule_event_t ));
		event->description = NULL;
		event->id = 2;
		parse_expression( &event->expression, "(true)", 6 );

		rule_node_t *rule_node = mmt_mem_alloc( sizeof( rule_node_t ));
		rule_node->type  = RULE_NODE_TYPE_EVENT;
		rule_node->event = event;
		rule.trigger = rule_node;
	} else {
	//when rule type != COMPUTE, we need 2 events: context and trigger
		ASSERT( (rule.context != NULL && rule.trigger != NULL ),
			"Error 13g: Require 2 <event> for context and trigger for rule %d", rule.id );
	}

	ret = mmt_mem_dup( &rule, sizeof( rule_t));
	return ret;
}

#define MAX_STRING_SIZE 500000
//TODO: this limit 100K rules
#define MAX_RULE_COUNT 100000
/**
 * Public API
 */
size_t read_rules_from_file( const char * file_name,  rule_t ***properties_arr, char **embedded_functions){
	xmlDoc *xml_doc = NULL;
	xmlNode *root_node = NULL, *xml_node;

	rule_t *array[MAX_RULE_COUNT], *rule_ptr ;
	char string[ MAX_STRING_SIZE ], *string_ptr;
	size_t rules_count = 0, size, string_size, i;
	xmlChar *xml_content;

	*properties_arr = NULL;
	//safe string
	string[ 0 ] = '\0';
	string_ptr = string;

	/*
	 * this initialize the library and check potential ABI mismatches
	 * between the version it was compiled for and the actual shared
	 * library used.
	 */
	LIBXML_TEST_VERSION

	//parse the file and get the DOM
	xml_doc = xmlReadFile(file_name, NULL, 0);
	ASSERT( xml_doc != NULL, "Error 13a: in XML properties file: %s. Parsing failed.\n", file_name );

	/*Get the root element node */
	root_node = xmlDocGetRootElement( xml_doc );

	ASSERT( root_node->type == XML_ELEMENT_NODE && str_equal(root_node->name, "beginning") ,
			"Error 13b: Name of the root node must be 'beginning', not '%s'", root_node->name );

	string_size = 0;
	//first property
	xml_node = root_node->children;
	while( xml_node != NULL ){

		if( xml_node->type == XML_ELEMENT_NODE ) {
			if( str_equal( xml_node->name, "property") ) {
				rule_ptr = _parse_a_rule( xml_node );

				//when we get a new property => increase the counter
				if( rule_ptr != NULL ){
					//check duplicated rule_id
					for( i=0; i<rules_count; i++ )
						ASSERT( array[i]->id != rule_ptr->id, "Error 13h: Duplicate rule id %d", rule_ptr->id );
					array[ rules_count ] = rule_ptr;
					if( rules_count >= MAX_RULE_COUNT -1 )
						log_write( LOG_WARNING, "Too much rules" );
					else
						rules_count ++;
				}
			}else if( str_equal( xml_node->name, "embedded_functions") ){
				//ASSERT( xml_node->type == XML_CDATA_SECTION_NODE, "Error 13b: Need to surround content of node \"%s\" by CDATA", xml_node->name );
				xml_content = xmlNodeGetContent( xml_node );

				size = snprintf( string_ptr, MAX_STRING_SIZE - string_size, "%s", (char *)xml_content );

				if( string_size + size >= MAX_STRING_SIZE )
					log_write( LOG_WARNING, "Embedded function is too long" );
				else{
					string_size += size;
					string_ptr += size;
					*string_ptr = '\0';
				}

				xmlFree( xml_content );
			}
		}
		//goto the next property
		xml_node = xml_node->next;
	}

	/*free the document */
	xmlFreeDoc( xml_doc );

	//Need to recuperate what attributes will need to be printed out (<proto_id, field_id, data_type_id>)

	// Cleanup function for the XML library.
	xmlCleanupParser();

	//copy result to a new array
	*properties_arr = mmt_mem_dup( &array, rules_count * sizeof( rule_t *) );


	*embedded_functions = mmt_mem_dup( string, string_size );

	return rules_count;
}

size_t _get_unique_events_of_rule_node( const rule_node_t *node, mmt_map_t *events_map, size_t *event_index ){
	size_t events_count = 0;
	rule_event_t *ptr;
	if ( node == NULL ) return 0;
	if( node->type == RULE_NODE_TYPE_EVENT ){

		//if user does not set id of event ==> we assign it to an unique number
		if( node->event->id == (uint16_t) UNKNOWN )
			node->event->id = *event_index;
		else{

		}

		(*event_index) ++;

		//check if event is existing in the map
		ptr = mmt_map_set_data( events_map, &(node->event->id), node->event, YES );
		//must not have 2 events with the same id
		ASSERT( ptr == NULL, "Error 13g: Duplicated events having id=%d", node->event->id );
		return 1;
	}else if( node->type == RULE_NODE_TYPE_OPERATOR ){
		events_count += _get_unique_events_of_rule_node( node->operator->context, events_map, event_index );
		events_count += _get_unique_events_of_rule_node( node->operator->trigger, events_map, event_index );
		return events_count;
	}else
		log_write( LOG_WARNING, "Unknown rule_node_t->type = %d", node->type );
	return 0;
}


static inline void _iterate_to_check_variables( void *key, void *data, void *user_arg, size_t index, size_t total ){
	mmt_map_t *map  = (mmt_map_t *)user_arg;
	variable_t *var = (variable_t *)data;
	void *ev_ptr = NULL;
	if( var->ref_index == (uint16_t)UNKNOWN )
		return;

	ev_ptr = mmt_map_get_data( map, &( var->ref_index) );
	ASSERT( ev_ptr != NULL, "Error 13h: Variable \"%s.%s.%d\" references to unknown event %d", var->proto, var->att, var->ref_index, var->ref_index );
}
static inline void _iterate_to_check_event_id_in_boolean_expression( void *key, void *data, void *user_arg, size_t index, size_t total ){
	rule_event_t *ev = (rule_event_t *) data;
	mmt_map_t *vars_map = NULL;

	//get list of variables
	get_unique_variables_of_expression( ev->expression, &vars_map, YES );
	//check
	mmt_map_iterate( vars_map, _iterate_to_check_variables, user_arg );
}

/**
 * Public API
 */
size_t get_unique_events_of_rule( const rule_t *rule, mmt_map_t **events_map ){
	size_t events_count = 0, event_index = 1;
	mmt_map_t *map = mmt_map_init( compare_uint16_t );

	events_count += _get_unique_events_of_rule_node( rule->context, map, &event_index );
	events_count += _get_unique_events_of_rule_node( rule->trigger, map, &event_index );
	if( events_count == 0 )
		mmt_free_and_assign_to_null( map );
	mmt_map_iterate( map, _iterate_to_check_event_id_in_boolean_expression, map );
	*events_map = map;

	if( events_count > 63 + 1 ) //real events + timeout
		ABORT("Cannot hold more than 64 events in a property");

	return events_count;
}
