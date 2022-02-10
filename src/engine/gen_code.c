/*
 * gen_code.c
 *
 *  Created on: 4 oct. 2016
 *  Created by: Huu Nghia NGUYEN <huunghia.nguyen@montimage.com>
 */

#include <mmt_core.h>
#include "mmt_fsm.h"
#include "gen_code.h"

#include "../lib/version.h"

#define STR_BUFFER_SIZE 10000
#define _gen_comment( fd, format, ... ) fprintf( fd, "\n/** %d\n * " format "\n */\n", __LINE__, ##__VA_ARGS__ )
#define _gen_comment_line( fd, format, ... ) fprintf( fd, "/** %d " format "*/", __LINE__, ##__VA_ARGS__ )
#define _gen_code_line( fd ) fprintf( fd, "/* %d */", __LINE__ )
#define _val( x ) (x==NULL? "NULL": x)
#define _num( x ) (x==NULL? 0: x)
#define _string( v, a,b,c, x,y,z  ) (v==NULL? a:x), (v==NULL? b:y), (v==NULL? c:z)



struct _user_data{
	uint16_t uint16_val;
	uint32_t uint32_val;
	mmt_map_t *map;
	FILE *file;
};


struct _variables_struct{
	char *proto, *att;
	uint32_t data_type, proto_id, att_id;
};


static void _iterate_variable( void *key, void *data, void *user_data, size_t index, size_t total ){
	char *str = NULL;
	struct _user_data *_u_data = (struct _user_data *)user_data;
	FILE *fd          = _u_data->file;
	uint32_t rule_id  = _u_data->uint32_val;
	size_t size;
	variable_t *var = (variable_t *) data;

	//init for the first element
	size = expr_stringify_variable( &str, var );
	if( size == 0 ) return;


	if( var->ref_index != (uint16_t)UNKNOWN ){
		fprintf(fd, "\n\t his_msg = fsm_get_history( fsm, %d );", var->ref_index );
		//TODO: not need to check ?
		fprintf(fd, "\n\t if( unlikely( his_msg == NULL )) return 0;");
	}

	_gen_code_line( fd );

	fprintf( fd, "\n\n\t data = get_element_data_message_t( %s, _m%"PRIu32"._%s_%s );",
					( var->ref_index == UNKNOWN_REF_INDEX )? "msg" : "his_msg",
					rule_id,
					var->proto, var->att
	);

	switch( var->data_type ){
	case MMT_SEC_MSG_DATA_TYPE_STRING:
		fprintf( fd, "\n\t const char *%s = (char *) data;", str );
		break;
	case MMT_SEC_MSG_DATA_TYPE_NUMERIC:
		fprintf( fd, "\n\t double %s = 0;", str );
		fprintf( fd, "\n\t if (data != NULL)  %s = *(double*) data;", str );
		break;
	default:
		fprintf( fd, "\n\t const void *%s = data;", str );
		break;
	}

	mmt_mem_free( str );
}


static void _iterate_event_to_gen_guards( void *key, void *data, void *user_data, size_t index, size_t total ){
	char *str = NULL, *guard_fun_name, buffer[1000];
	size_t size, var_count;
	mmt_map_t *map;
	rule_event_t *event = (rule_event_t *)data;
	struct _user_data *_u_data = (struct _user_data *)user_data;
	FILE *fd          = _u_data->file;
	uint32_t rule_id  = _u_data->uint32_val;
	uint16_t event_id = event->id;

	_gen_comment( fd, "Rule %"PRIu32", event %d\n  * %s", rule_id, event_id, event->description == NULL? "" : event->description );
	size = expr_stringify_expression( &str, event->expression );
	if( size == 0 ) return;

	//set of variables
	var_count = get_unique_variables_of_expression( event->expression, &map, YES );

	//name of function
	size = snprintf( buffer, sizeof( buffer ) -1, "g_%"PRIu32"_%"PRIu32"", rule_id, event_id );
	//do not free this as it will be used as a key in variables_map
	guard_fun_name = mmt_mem_dup( buffer, size );

	//guard function header
	fprintf(fd, "static inline int %s( const message_t *msg, const fsm_t *fsm ){",
			guard_fun_name );
	if( var_count != 0 ){
		fprintf(fd, "\n\t if( unlikely( msg == NULL || fsm == NULL )) return 0;" );
		fprintf(fd, "\n\t const message_t *his_msg;");
		fprintf(fd, "\n\t const void *data;");
		mmt_map_iterate( map, _iterate_variable, user_data );
	}
	fprintf(fd, "\n\n\t return %s;\n }\n ",  str);

	mmt_mem_free( str );
	//add events to list events
	//mmt_map_set_data(u_data->events_map, guard_fun_name, map, NO);
	mmt_map_free( map, NO );
	mmt_mem_free( guard_fun_name );
}

////////////////////////////////////////////////////////////////////////////////
//Gen FSM from a rule///////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#define MAX_STR_BUFFER 10000
/**
 * Meta-model
 * This structure contains information to generate fsm_state_t
 */
typedef struct _meta_state_struct{
	size_t index;
	char *description;
	rule_delay_t *delay;
	link_node_t *transitions;
	char comment[MAX_STR_BUFFER];
	int entry_action;
	int exit_action;
}_meta_state_t;
/**
 * Meta-model
 * This structure contains information to generate fsm_transition_t
 */
typedef struct _meta_transition_struct{
	int event_type;
	int guard_id;
	_meta_state_t *target;
	const rule_event_t *attached_event;//
	char comment[MAX_STR_BUFFER];
	int action;
}_meta_transition_t;

static inline _meta_state_t *_create_new_state( size_t index ){
	_meta_state_t *s = mmt_mem_alloc( sizeof( _meta_state_t ));
	s->index        = index;
	s->description  = NULL;
	s->delay        = NULL;
	s->transitions  = NULL;
	s->entry_action = FSM_ACTION_DO_NOTHING;
	s->exit_action  = FSM_ACTION_DO_NOTHING;
	s->comment[0]   = '\0';
	return s;
}

static inline _meta_transition_t *_create_new_transition( int event_type, int guard_id, _meta_state_t *target, int action, const rule_event_t *ev, const char *comment){
	_meta_transition_t *t = mmt_mem_alloc( sizeof( _meta_transition_t ));
	t->event_type = event_type;
	t->guard_id   = guard_id;
	t->target     = target;
	t->action     = action;
	t->attached_event = ev;
	if( comment != NULL )
		snprintf(t->comment, MAX_STR_BUFFER, "%s", comment);
	else
		t->comment[0] = '\0';
	return t;
}

static inline void _gen_transition_rule( _meta_state_t *s_init, _meta_state_t *s_pass,  _meta_state_t *s_fail, _meta_state_t *s_incl,
		link_node_t *states_list, const  rule_node_t *rule_node, size_t *index,  const rule_t *rule, int tran_action);


/**
 * a THEN b
 * target state of a is the source state of b
 */
static inline void _gen_transition_then( _meta_state_t *s_init,  _meta_state_t *s_pass,  _meta_state_t *s_fail, _meta_state_t *s_incl,
		link_node_t *states_list, const  rule_node_t *context, const  rule_node_t *trigger,
		size_t *index, const rule_operator_t *operator, const rule_t *rule, int tran_action ){
	_meta_state_t *new_state = _create_new_state( 0 );
	//root
	if( operator == NULL ){
		new_state->description = rule->description;
		new_state->delay       = rule->delay;
		snprintf(new_state->comment, MAX_STR_BUFFER, "root node");
	}else{
		new_state->description = operator->description;
		new_state->delay       = operator->delay;
	}

	//add timeout transition ==> goto error state
		new_state->transitions = append_node_to_link_list( new_state->transitions,
			_create_new_transition( FSM_EVENT_TYPE_TIMEOUT, 0, s_fail, FSM_ACTION_DO_NOTHING, NULL, "Timeout event will fire this transition"));

	//gen for context
	//if context is not satisfied => goto #s_incl instead of #s_fail
	_gen_transition_rule( s_init, new_state, s_incl, s_incl, states_list, context, index, rule, FSM_ACTION_CREATE_INSTANCE );
	//increase index
	new_state->index = (*index)++;
	states_list  = append_node_to_link_list( states_list, new_state );
	//gen for trigger
	_gen_transition_rule( new_state, s_pass, s_fail, s_fail, states_list, trigger, index, rule, FSM_ACTION_RESET_TIMER );
}

/**
 * a AND b == (a THEN b) OR (b THEN a)
 */
static inline void _gen_transition_and( _meta_state_t *s_init,  _meta_state_t *s_pass,  _meta_state_t *s_fail, _meta_state_t *s_incl,
		link_node_t *states_list, const  rule_node_t *context, const  rule_node_t *trigger,
		size_t *index, const rule_operator_t *operator, const rule_t *rule, int tran_action ){

	_gen_transition_then( s_init, s_pass, s_fail, s_incl, states_list, context, trigger, index, operator, rule, tran_action );
	_gen_transition_then( s_init, s_pass, s_fail, s_incl, states_list, trigger, context, index, operator, rule, tran_action );
}

/**
 * a OR b
 * a and b having the same source and target states
 */
static inline void _gen_transition_or( _meta_state_t *s_init,  _meta_state_t *s_pass,  _meta_state_t *s_fail, _meta_state_t *s_incl,
		link_node_t *states_list, const  rule_node_t *context, const  rule_node_t *trigger,
		size_t *index, const rule_operator_t *operator, const rule_t *rule, int tran_action ){

	_gen_transition_rule( s_init, s_pass, s_fail, s_incl, states_list, context, index, rule, tran_action );
	_gen_transition_rule( s_init, s_pass, s_fail, s_incl, states_list, trigger, index, rule, tran_action );
}

/**
 * a NOT b
 *
 */
static inline void _gen_transition_not( _meta_state_t *s_init,  _meta_state_t *s_pass,  _meta_state_t *s_fail, _meta_state_t *s_incl,
		link_node_t *states_list, const  rule_node_t *context, const  rule_node_t *trigger,
		size_t *index, const rule_operator_t *operator, const rule_t *rule, int tran_action ){

	_meta_state_t *state = _create_new_state( 0 );
	//root
	if( operator == NULL ){
		state->description = rule->description;
		state->delay       = rule->delay;
		snprintf(state->comment, MAX_STR_BUFFER, "root node");
	}else{
		state->description = operator->description;
		state->delay       = operator->delay;
	}


	//add timeout transition
		state->transitions = append_node_to_link_list( state->transitions,
			_create_new_transition( FSM_EVENT_TYPE_TIMEOUT, 0, s_pass, FSM_ACTION_DO_NOTHING, NULL, "Timeout event will fire this transition"));

	//gen for context
	_gen_transition_rule( s_init, state, s_incl, s_incl, states_list, context, index, rule, FSM_ACTION_CREATE_INSTANCE );
	//increase index
	state->index = (*index)++;
	states_list  = append_node_to_link_list( states_list, state );
	//gen for trigger
	//if #trigger occurs => goto #s_fail, otherwise if timeout => goto #s_pass
	_gen_transition_rule( state, s_fail, s_pass, s_fail, states_list, trigger, index, rule, tran_action );
}

/**
 * a BEFORE b : if we have a, then b must be occurred before
 *
 */
static inline void _gen_transition_before( _meta_state_t *s_init,  _meta_state_t *s_pass,  _meta_state_t *s_fail, _meta_state_t *s_incl,
		link_node_t *states_list, const  rule_node_t *context, const  rule_node_t *trigger,
		size_t *index, const rule_operator_t *operator, const rule_t *rule, int tran_action ){

	_meta_state_t *state = _create_new_state( 0 );
	//root
	if( operator == NULL ){
		state->description = rule->description;
		state->delay       = rule->delay;
		snprintf(state->comment, MAX_STR_BUFFER, "root node");
	}else{
		state->description = operator->description;
		state->delay       = operator->delay;
	}

	//add timeout transition
		state->transitions = append_node_to_link_list( state->transitions,
			_create_new_transition( FSM_EVENT_TYPE_TIMEOUT, 0, s_incl, FSM_ACTION_DO_NOTHING, NULL, "Timeout event will fire this transition"));

	//if a occurred but b did not occurs before => goto #s_fail
	_gen_transition_rule( s_init, s_fail, s_fail, s_incl, states_list, context, index, rule, tran_action );

	//gen for b that must occurs before
	_gen_transition_rule( s_init, state, s_incl, s_incl, states_list, trigger, index, rule, FSM_ACTION_CREATE_INSTANCE );
	//increase index
	state->index = (*index)++;
	states_list  = append_node_to_link_list( states_list, state );
	//gen for a that occurs after b
	//if a (e.g., and b before) occurs => goto #s_pass,
	// otherwise if timeout (e.g., we had b but not a) => goto #s_incl
	_gen_transition_rule( state, s_pass, s_incl, s_incl, states_list, context, index, rule, tran_action );
}

/**
 * Generate a fsm of a rule
 */
static inline void _gen_transition_rule( _meta_state_t *s_init,  _meta_state_t *s_pass,  _meta_state_t *s_fail, _meta_state_t *s_incl,
		link_node_t *states_list, const  rule_node_t *rule_node, size_t *index, const rule_t *rule, int tran_action ){

	rule_operator_t *opt;
	__check_null( rule_node, );

	if( rule_node->type == RULE_NODE_TYPE_EVENT ){
		s_init->transitions = append_node_to_link_list( s_init->transitions,
				_create_new_transition(rule_node->event->id, rule_node->event->id, s_pass,  tran_action, rule_node->event, "A real event" ));
		return;
	}

	opt = rule_node->operator;
	switch( opt->value){
	case RULE_VALUE_THEN:
		_gen_transition_then(s_init, s_pass, s_fail, s_incl, states_list, opt->context, opt->trigger, index, opt, rule, tran_action);
		break;
	case RULE_VALUE_AND:
		_gen_transition_and(s_init, s_pass, s_fail, s_incl, states_list, opt->context, opt->trigger, index, opt, rule, tran_action);
		break;
	case RULE_VALUE_OR:
		_gen_transition_or(s_init, s_pass, s_fail, s_incl, states_list, opt->context, opt->trigger, index, opt, rule, tran_action);
		break;
	case RULE_VALUE_NOT:
		_gen_transition_not(s_init, s_pass, s_fail, s_incl, states_list, opt->context, opt->trigger, index, opt, rule, tran_action);
		break;
	case RULE_VALUE_COMPUTE:
		break;
	case RULE_VALUE_BEFORE:
		_gen_transition_before(s_init, s_pass, s_fail, s_incl, states_list, rule->context, rule->trigger, index, NULL, rule, FSM_ACTION_DO_NOTHING);
		break;
	default:
		ABORT( "Does not support value=%d of operator tag.", opt->value );
	}
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static void _gen_fsm_state_for_a_rule( FILE *fd, const rule_t *rule ){
	size_t size, states_count;
	_meta_state_t *s_init, *s_fail, *s_pass, *s_incl, *state;
	_meta_transition_t *tran;
	link_node_t *states_list = NULL, *p_link_node, *p_t;
	char buffer[ MAX_STR_BUFFER ];

	uint32_t rule_id = rule->id;

	static const char *fsm_action_string[] = {
			"FSM_ACTION_DO_NOTHING",      //0
			"FSM_ACTION_CREATE_INSTANCE", //1
			"FSM_ACTION_RESET_TIMER",     //2
			"FSM_ACTION_CREATE_INSTANCE | FSM_ACTION_RESET_TIMER" //3
	};

	states_count = 0;
	s_init = _create_new_state( states_count ++ );
	s_fail = _create_new_state( states_count ++ );
	s_pass = _create_new_state( states_count ++ );
	s_incl = _create_new_state( states_count ++ );

	sprintf(s_init->comment, "initial state");
	s_init->description = rule->description;
	s_init->exit_action = FSM_ACTION_CREATE_INSTANCE;

	sprintf(s_pass->comment, "pass state");
	//s_pass->entry_action = rule->if_satisfied;

	sprintf( s_fail->comment, "timeout/error state");
	//s_fail->entry_action = rule->if_not_satisfied;

	sprintf( s_incl->comment, "inconclusive state");

	states_list = append_node_to_link_list(states_list, s_init );
	states_list = append_node_to_link_list(states_list, s_fail );
	states_list = append_node_to_link_list(states_list, s_pass );
	states_list = append_node_to_link_list(states_list, s_incl );

	switch( rule->value){
	case RULE_VALUE_THEN:
		_gen_transition_then(s_init, s_pass, s_fail, s_incl, states_list, rule->context, rule->trigger, &states_count, NULL, rule, FSM_ACTION_DO_NOTHING);
		break;
	case RULE_VALUE_AND:
		_gen_transition_and(s_init, s_pass, s_fail, s_incl, states_list, rule->context, rule->trigger, &states_count, NULL, rule, FSM_ACTION_DO_NOTHING);
		break;
	case RULE_VALUE_OR:
		_gen_transition_or(s_init, s_pass, s_fail, s_incl, states_list, rule->context, rule->trigger, &states_count, NULL, rule, FSM_ACTION_DO_NOTHING);
		break;
	case RULE_VALUE_NOT:
		_gen_transition_not(s_init, s_pass, s_fail, s_incl, states_list, rule->context, rule->trigger, &states_count, NULL, rule, FSM_ACTION_DO_NOTHING);
		break;
	case RULE_VALUE_COMPUTE:
		break;
	case RULE_VALUE_BEFORE:
		_gen_transition_before(s_init, s_pass, s_fail, s_incl, states_list, rule->context, rule->trigger, &states_count, NULL, rule, FSM_ACTION_DO_NOTHING);
		break;
	default:
		ABORT( "Does not support value=%d of property tag.", rule->value );
	}

	_gen_comment( fd, "States of FSM for rule %"PRIu32"", rule_id );
	_gen_comment(fd, "Predefine list of states: init, fail, pass, ..." );
	fprintf(fd, "static fsm_state_t");
	p_link_node = states_list;
	while( p_link_node != NULL ){
		state = (_meta_state_t *)p_link_node->data;
		fprintf( fd, " s_%"PRIu32"_%zu%c", rule_id, state->index, (p_link_node->next == NULL? ';':',') );

		p_link_node = p_link_node->next;
	}

	/**
	 * Print detail of each state
	 */
	_gen_comment(fd, "Initialize states: init, error, final, ..." );
	fprintf(fd, "static fsm_state_t");
	p_link_node = states_list;
	while( p_link_node != NULL ){
		state = (_meta_state_t *)p_link_node->data;
		if( strlen( state->comment ))
			_gen_comment(fd, "%s", state->comment );

		fprintf( fd, " s_%"PRIu32"_%zu = {", rule_id, state->index );
		if( state->delay == NULL ){
			fprintf( fd, "\n\t .delay        = {.time_min = 0, .time_max = 0, .counter_min = 0, .counter_max = 0},");
			fprintf( fd, "\n\t .is_temporary = 0," );
		}else{
			fprintf( fd, "\n\t .delay        = {.time_min = %"PRIu64"LL, .time_max = %"PRIu64"LL, .counter_min = %"PRIu64"LL, .counter_max = %"PRIu64"LL},",
					state->delay->time_min,    state->delay->time_max,
					state->delay->counter_min, state->delay->counter_max);

			fprintf( fd, "\n\t .is_temporary = %d,", !!(state->delay->time_max == 0 && state->delay->time_min == 0
													  	  && state->delay->counter_max == 0 && state->delay->counter_min == 0));
		}
		fprintf( fd, "\n\t .description  = %c%s%c,", _string( state->description, ' ', "NULL", ' ', '"', state->description, '"'));
		fprintf( fd, "\n\t .entry_action = %d, //%s", state->entry_action, fsm_action_string[ state->entry_action ] );
		fprintf( fd, "\n\t .exit_action  = %d, //%s", state->exit_action,  fsm_action_string[ state->exit_action ] );
		size = 0;
		//print list of outgoing transitions of this state
		if( state->transitions == NULL ){
			fprintf( fd, "\n\t .transitions  = NULL,");
		}else{
			fprintf( fd, "\n\t .transitions  = (fsm_transition_t[]){");
			p_t = state->transitions;
			while( p_t != NULL ){
				size ++;
				tran = (_meta_transition_t *)p_t->data;
				if( tran->attached_event && tran->attached_event->description )
					fprintf( fd, "\n\t\t /** %d %s */", __LINE__, tran->attached_event->description );
				if( tran->comment[0] != '\0' )
					fprintf( fd, "\n\t\t /** %d %s */", __LINE__, tran->comment );
				sprintf( buffer, "&g_%"PRIu32"_%"PRIu32"", rule_id, tran->guard_id );

				//always create a new instance when going out from the initial state
				if( state == s_init )
					tran->action = FSM_ACTION_CREATE_INSTANCE;

				fprintf( fd, "\n\t\t { .event_type = %d, .guard = %s, .action = %"PRIu32", .target_state = &s_%d_%zu}%c //%s",
						tran->event_type,
						(tran->event_type == FSM_EVENT_TYPE_TIMEOUT ? "NULL  "  : buffer   ), //guard
						tran->action,
						rule_id, tran->target->index, //target_state
						(p_t->next == NULL?' ':','),
						fsm_action_string[ tran->action ]
				);
				p_t = p_t->next;
			}
			fprintf( fd, "\n\t },");
			free_link_list( state->transitions, YES );
		}

		fprintf( fd, "\n\t .transitions_count = %zu", size );

		fprintf( fd, "\n }%c", (p_link_node->next == NULL? ';':',') );
		p_link_node = p_link_node->next;
	}

	_gen_comment(fd, "Create a new FSM for this rule");
	fprintf( fd, "static void *create_new_fsm_%d(){", rule->id);
	fprintf( fd, "\n\t\t return fsm_init( &s_%d_%zu, &s_%d_%zu, &s_%d_%zu, &s_%d_%zu, EVENTS_COUNT_%d, sizeof( _msg_t_%d ) );//init, error, final, inconclusive, events_count",
			rule->id, s_init->index, rule->id, s_fail->index, rule->id, s_pass->index, rule->id, s_incl->index, rule->id, rule->id );
	fprintf( fd, "\n }//end function");

	free_link_list( states_list, YES );
}

void _iterate_variable_to_print_hash_function_body( void *key, void *data, void *user_data, size_t index, size_t total){
	FILE *fd         = (FILE *) user_data;
	variable_t  *var = (variable_t *) data;

	if( index == 0 )
		fprintf( fd, "\n\t if( msg->%s_%s != NULL", var->proto, var->att );
	else
		fprintf( fd, " && msg->%s_%s != NULL", var->proto, var->att );

	if( index == total - 1 )
		fprintf( fd, " )");
}

void _iterate_variables_to_gen_structure( void *key, void *data, void *user_data, size_t index, size_t total ){
	struct _user_data *_u_data = (struct _user_data *)user_data;
	FILE *fd         = _u_data->file;
	uint32_t rule_id = _u_data->uint32_val;
	variable_t *var  = (variable_t *)data;

	//first element
	if( index == 0 ){
		_gen_comment( fd, "Structure to represent event data");
		fprintf( fd, "typedef struct _msg_struct_%"PRIu32"{", rule_id );
	}
	//first underscore, before proto, ensures to cover the case that proto_name starts by a number
	fprintf( fd, "\n\t uint16_t _%s_%s;", var->proto, var->att);

	//last element
	if( index + 1 == total ){
		fprintf( fd, "\n }_msg_t_%"PRIu32";", rule_id );
	}
}

void _iterate_variables_to_gen_array_proto_att( void *key, void *data, void *user_data, size_t index, size_t total ){
	FILE *fd = (FILE *)user_data;
	variable_t *var = (variable_t *)data;

	fprintf( fd, "%c{.proto = \"%s\", .proto_id = %"PRIu32", .att = \"%s\", .att_id = %"PRIu32", .data_type = %d, .dpi_type = %d}%s",
			index == 0 ? '{':' ',
			var->proto, var->proto_id,
			var->att, var->att_id,
			var->data_type,
			var->dpi_type,
			index == total-1? "}":",\n"
	);
}

void _iterate_variables_to_init_structure( void *key, void *data, void *user_data, size_t index, size_t total ){
	struct _user_data *_u_data = (struct _user_data *)user_data;
	FILE *fd         = _u_data->file;
	uint32_t rule_id = _u_data->uint32_val;
	variable_t *var  = (variable_t *)data;

	//first element
	if( index == 0 ){
		_gen_comment( fd, "Create an instance of _msg_t_%"PRIu32"", rule_id);
		fprintf( fd, "static _msg_t_%"PRIu32" _m%"PRIu32";", rule_id, rule_id );
		fprintf( fd, "\n static void _allocate_msg_t_%"PRIu32"( const char* proto, const char* att, uint16_t index ){", rule_id );
	}

	fprintf( fd, "\n\t if( strcmp( proto, \"%s\" ) == 0 && strcmp( att, \"%s\" ) == 0 ){ _m%"PRIu32"._%s_%s = index; return; }",
			var->proto, var->att,
			rule_id,
			var->proto, var->att );

	//last element
	if( index + 1 == total ){
		fprintf( fd, "\n }" );
	}
}

static inline bool _is_embedded_function_name( const char *str ){
	if( str && str[0] == '#' )
		return true;
	return false;
}

static inline void _get_if_statisfied_function_name( const rule_t *rule, char *buffer, size_t buffer_size ){
	uint16_t rule_id = rule->id;
	const char*fn_name = rule->if_satisfied;

	ASSERT( buffer_size >= 50, "Need at least 5 characters to contain if_satisified name (must not happen)" );
	if( fn_name == NULL ){
		//when FORWARD, default function name is _forward_packet to forward packet
		if( rule->type == RULE_TYPE_FORWARD )
			snprintf( buffer, buffer_size, "forward_packet_if_satisfied" );
		else
			snprintf( buffer, buffer_size, "NULL" );
		return;
	}
	//a function name to be called
	if( ! _is_embedded_function_name( fn_name ) ){
		if( buffer_size > strlen( fn_name ))
			buffer_size = strlen( fn_name ) + 1; //+1: the \0 character at the end

		snprintf( buffer, buffer_size, "%s", fn_name );
		return;
	}
	//an embedded function, e.g., #update_data( PROTO_S1AP, RAN_UE_ID, s1ap.ran_ue_id.1 + 1 )
	//=> we need to create an unique function for this rule
	snprintf( buffer, buffer_size, "_fn_if_satisified_%d", rule->id );

	return;
}

/**
 * Generate general informations of rules
 */
static inline void _gen_rule_information( FILE *fd, rule_t *const* rules, size_t count ){
	size_t i;
	char *string;
	char new_fn_name[255];

	_gen_comment(fd, "Moment the rules being encoded\n  * PUBLIC API");
	fprintf( fd, "\nstatic const rule_version_info_t version = {.created_date=%ld, .hash = \"%s\", .number=\"%s\", .index=%d, .dpi=\"%s\"};",
					time( NULL ),
					mmt_sec_get_version_hash(),
					mmt_sec_get_version_number(),
					mmt_sec_get_version_index(),
					mmt_version() //dpi
					);

	fprintf( fd, "\nconst rule_version_info_t * mmt_sec_get_rule_version_info(){ return &version;};" );

	//fprintf( fd, "\ntypedef struct fun_struct{const char* name, mmt_rule_satisfied_callback *func; }fun_t;");

	fprintf(fd, "\n\n //======================================GENERAL======================================");
	_gen_comment( fd, "Information of %zu rules\n  * PUBLIC API", count );
	fprintf( fd, "size_t mmt_sec_get_plugin_info( const rule_info_t **rules_arr ){");
	fprintf( fd, "\n\t  static const rule_info_t rules[] = (rule_info_t[]){");
	for( i=0; i<count; i++ ){
		_get_if_statisfied_function_name( rules[i], new_fn_name, sizeof( new_fn_name));
		fprintf( fd, "\n\t\t {");
		fprintf( fd, "\n\t\t\t .id               = %d,", rules[i]->id );
		fprintf( fd, "\n\t\t\t .type_id          = %d,", rules[i]->type );
		fprintf( fd, "\n\t\t\t .type_string      = \"%s\",", rule_type_string[ rules[i]->type ] );
		fprintf( fd, "\n\t\t\t .events_count     = EVENTS_COUNT_%d,", rules[i]->id );
		fprintf( fd, "\n\t\t\t .description      = %c%s%c,",
				_string( rules[i]->description, 'N', "UL", 'L', '"', rules[i]->description, '"') );

		fprintf( fd, "\n\t\t\t .if_satisfied     = %s,", new_fn_name );
		fprintf( fd, "\n\t\t\t .if_not_satisfied = %c%s%c,",
				_string( rules[i]->if_not_satisfied, 'N', "UL", 'L', '"', rules[i]->if_not_satisfied, '"') );

		fprintf( fd, "\n\t\t\t .proto_atts_count = PROTO_ATTS_COUNT_%d,", rules[i]->id );
		fprintf( fd, "\n\t\t\t .proto_atts       = proto_atts_%d,", rules[i]->id );
		fprintf( fd, "\n\t\t\t .proto_atts_events= proto_atts_events_%d,", rules[i]->id );
		fprintf( fd, "\n\t\t\t .excluded_filter  = excluded_filter_%d,", rules[i]->id );


		fprintf( fd, "\n\t\t\t .create_instance  = &create_new_fsm_%d,", rules[i]->id );
		fprintf( fd, "\n\t\t\t .hash_message     = &_allocate_msg_t_%d,", rules[i]->id );
		fprintf( fd, "\n\t\t\t .version          = &version,");


		if( i < count -1 )
			fprintf( fd, "\n\t\t },");
		else
			fprintf( fd, "\n\t\t }");
	}
	fprintf( fd, "\n\t };\n\t *rules_arr = rules;");
	fprintf( fd, "\n\t return %zu;\n }", count);
}


static inline void _iterate_variable_to_add_to_a_new_map( void *key, void *data, void *user_data, size_t index, size_t total ){
	mmt_map_set_data((mmt_map_t *)user_data, data, data, NO );
}
static inline void _iterate_event_to_get_unique_variables( void *key, void *data, void *user_data, size_t index, size_t total ){
	rule_event_t *ev = (rule_event_t *)data;
	mmt_map_t *map;

	get_unique_variables_of_expression( ev->expression, &map, NO );
	mmt_map_iterate( map, _iterate_variable_to_add_to_a_new_map, user_data );
	mmt_map_free( map, NO );
}


void _iterate_variables_to_gen_pointer_proto_att( void *key, void *data, void *user_data, size_t index, size_t total ){
	struct _user_data *u_data = (struct _user_data *) user_data;
	mmt_map_t *variables_map  = (mmt_map_t *)  u_data->map;
	variable_t *var           = (variable_t *) data;

	int val = mmt_map_get_index( variables_map, var );

	ASSERT( val >= 0, "Variable %s.%s must be defined before!", var->proto, var->att );

	fprintf( u_data->file, "%c &proto_atts_%d[ %"PRIu32" ] %s",
			index == 0 ? '{':' ',
			u_data->uint32_val,
			val,
			index == total-1? "}":","
	);
}

static inline void _iterate_events_to_gen_array_proto_att( void *key, void *data, void *user_data, size_t index, size_t total ){
	struct _user_data *u_data = (struct _user_data *)user_data;
	mmt_map_t *variables_map = (mmt_map_t *)data;
	uint16_t event_id = * (uint16_t *)key;
	size_t variables_count = mmt_map_count( variables_map);

	//each event
	fprintf( u_data->file, "\n\t {//event_%d", event_id );

	//variables of each event
	fprintf( u_data->file, "\n\t\t .elements_count = %zu,", variables_count );


	if( variables_count > 0 ){
		fprintf( u_data->file, "\n\t\t .data = (void* []) " );
		mmt_map_iterate( variables_map, _iterate_variables_to_gen_pointer_proto_att, user_data );
	}else
		fprintf( u_data->file, "\n\t\t .data = NULL" );
	fprintf( u_data->file, "\n\t }%c", index + 1 == total ? ' ':',' );
}


static inline void _get_excluded_proto_atts( expression_t *expr, mmt_map_t *result, bool to_add ){
	const link_node_t *node;

	if( expr->type == VARIABLE && to_add == YES ){
		mmt_map_set_data( result, expr->variable, expr->variable, NO );
		return;
	}

	if( expr->type != OPERATION )
		return;

	if( expr->operation->operator == FUNCTION && strcmp( expr->operation->name, "is_exist") == 0 )
		to_add = YES;

	//check for each parameter of the operation
	for( node=expr->operation->params_list; node != NULL; node = node->next )
		_get_excluded_proto_atts( (expression_t *) node->data, result, to_add );
}

static inline void _iterate_events_to_gen_excluded_proto_att( void *key, void *data, void *user_data, size_t index, size_t total ){
	struct _user_data *u_data = (struct _user_data *)user_data;
	const rule_event_t *rule_ev = (rule_event_t *)data;
	uint16_t event_id = * (uint16_t *)key;

	mmt_map_t *exclude_variables_map = mmt_map_init( compare_variable_name );

	_get_excluded_proto_atts( rule_ev->expression, exclude_variables_map, NO );

	size_t variables_count = mmt_map_count( exclude_variables_map );

	//each event
	fprintf( u_data->file, "\n\t {//event_%d", event_id );

	//variables of each event
	fprintf( u_data->file, "\n\t\t .elements_count = %zu,", variables_count );

	if( variables_count > 0 ){
		fprintf( u_data->file, "\n\t\t .data = (void* []) " );
		mmt_map_iterate( exclude_variables_map, _iterate_variables_to_gen_pointer_proto_att, user_data );
	}else
		fprintf( u_data->file, "\n\t\t .data = NULL" );

	fprintf( u_data->file, "\n\t }%c", index + 1 == total ? ' ':',' );
	mmt_map_free( exclude_variables_map, NO );
}

static inline void _iterate_event_to_verify_id( void *key, void *data, void *user_data, size_t index, size_t total ){
	struct _user_data *_u_data = (struct _user_data *)user_data;
	uint32_t rule_id = _u_data->uint32_val;
	rule_event_t *ev = (rule_event_t *)data;

	ASSERT( ev->id <= total, "Error in rule %d: Event_id %d is greater than number of events (%zu). Event_id must start from 1 and continue ..",
			rule_id, ev->id, total );
	ASSERT( ev->id > 0, "Error in rule %d: Event_id must start from 1, not 0.", rule_id );
}


static inline void _iterate_variable_to_add_to_a_new_map_2( void *key, void *data, void *user_data, size_t index, size_t total ){
	struct _user_data *_u_data = (struct _user_data *) user_data;
	variable_t *var = (variable_t *) data;
	uint16_t ref = var->ref_index;
	mmt_map_t *map;

	//in a case, variable does not reference to any event => it belongs to the current event
	if( ref == UNKNOWN_REF_INDEX )
		ref = _u_data->uint16_val;

	map = mmt_map_get_data( _u_data->map, &ref );

	//create a new entry to store all variables belonging to an event
	if( map == NULL ){
		map = mmt_map_init( compare_variable_name );
		mmt_map_set_data( _u_data->map, mmt_mem_dup( &ref, sizeof( ref ) ), map, NO );
	}

	mmt_map_set_data( map, var, var, NO );
}
static inline void _iterate_events_to_get_variables_of_each_event( void *key, void *data, void *user_data, size_t index, size_t total ){
	rule_event_t *ev = (rule_event_t *)data;
	mmt_map_t *map; //variables of event #ev
	size_t var_count;
	uint16_t *ev_id_ptr;
	struct _user_data _u_data;
	_u_data.uint16_val = ev->id;
	_u_data.map = (mmt_map_t *)user_data;

	//map stores a set of variables using in event #ev
	var_count = get_unique_variables_of_expression( ev->expression, &map, YES );

	//add the variables to one or more linked-lists of events the variables belong to
	//e.g., event #ev may use 2 variables in its boolean expression: ( ip.src == ip.dst.2 )
	// => the variable ip.src belongs to #ev
	//    but the variable ip.dst belongs to an event having id = 2
	if( var_count > 0 )
		mmt_map_iterate( map, _iterate_variable_to_add_to_a_new_map_2, &_u_data );
	else{
		//this event has no variable
		//insert a dummy
		if( mmt_map_get_data( _u_data.map, &(ev->id) ) == NULL )
			mmt_map_set_data( _u_data.map, mmt_mem_dup( &(ev->id), sizeof( ev->id) ), mmt_map_init( compare_variable_name ), NO );
	}

	mmt_map_free( map, NO );
}

static inline void _free_a_map( void *v ){
	mmt_map_free( (mmt_map_t *) v, NO );
}

static inline void _gen_fsm_for_a_rule( FILE *fd, const rule_t *rule ){
	size_t events_count, variables_count;
	struct _user_data _u_data;
	_u_data.file       = fd;
	_u_data.uint32_val = rule->id;
	/**
	 * a set of events
	 * <event_id, map>
	 */
	mmt_map_t *events_map = NULL;
	/**
	 * a set of unique variables
	 * <variable,variable>
	 */
	mmt_map_t *variables_map =  mmt_map_init( compare_variable_name );

	/**
	 * List of variables (proto, att) need by each event,
	 * e.g., if we have boolean_expression of event 1: (ip.src == ip.dst.2)
	 *   then, #event_variables_map will contain 2 entries:
	 *     - for event1: ip.src
	 *     - for event2: ip.dst
	 * <event, map_of_variables>
	 */
	mmt_map_t *event_variables_map = mmt_map_init( compare_uint16_t );


	events_count = get_unique_events_of_rule( rule, &events_map );
	if( events_count == 0 ) return;

	mmt_map_iterate( events_map, _iterate_event_to_verify_id, &_u_data );

	mmt_map_iterate( events_map, _iterate_event_to_get_unique_variables, variables_map );

	variables_count = mmt_map_count( variables_map );

	fprintf( fd, "\n\n //======================================RULE %d======================================", rule->id );
	fprintf( fd, "\n #define EVENTS_COUNT_%d %zu\n", rule->id, events_count );
	fprintf( fd, "\n #define PROTO_ATTS_COUNT_%d %zu\n", rule->id, variables_count );

	_gen_comment( fd, "Proto_atts for rule %d", rule->id );
	fprintf( fd, "\n static proto_attribute_t proto_atts_%d[ PROTO_ATTS_COUNT_%d ] = ", rule->id, rule->id );
	if( variables_count > 0 )
		mmt_map_iterate(variables_map, _iterate_variables_to_gen_array_proto_att, fd );
	else
		//there is no variables
		fprintf( fd, "{}");
	fprintf( fd, ";");

	//get a set of variables for each event
	mmt_map_iterate( events_map, _iterate_events_to_get_variables_of_each_event, event_variables_map );

	_gen_comment( fd, "Detail of proto_atts for each event");
	//mmt_map_iterate( event_variables_map, _iterate_events_to_gen_proto_att, &_u_data );

	//first element having index = 0 is null as event_id starts from 1
	fprintf( fd, "\n static mmt_array_t proto_atts_events_%d[ %zu ] = { {.elements_count = 0, .data = NULL}, ",
			rule->id, events_count + 1 );
	_u_data.map = variables_map;
	mmt_map_iterate(event_variables_map, _iterate_events_to_gen_array_proto_att, &_u_data );
	fprintf( fd, "\n };//end proto_atts_events_\n" );

	fprintf( fd, "\n static mmt_array_t excluded_filter_%d[ %zu ] = { {.elements_count = 0, .data = NULL}, ",
			rule->id, events_count + 1 );
	_u_data.map = variables_map;
	mmt_map_iterate(events_map, _iterate_events_to_gen_excluded_proto_att, &_u_data );
	fprintf( fd, "\n };//end excluded_filter_\n" );

	//define a structure using in guard functions
	if( variables_count > 0 )
		mmt_map_iterate(variables_map, _iterate_variables_to_gen_structure, &_u_data );
	else
		fprintf( fd, "\n typedef void _msg_t_%d;", rule->id );

	//convert from a message_t to a structure generated above
	if( variables_count > 0 )
		mmt_map_iterate(variables_map, _iterate_variables_to_init_structure, &_u_data );
	else
		fprintf( fd, "\n void _allocate_msg_t_%d(const char* proto, const char* att, uint16_t index){ }", rule->id );

	mmt_map_iterate(events_map, _iterate_event_to_gen_guards, &_u_data );

	_gen_fsm_state_for_a_rule( fd, rule );

	//free mmt_map
	mmt_map_free_key_and_data( event_variables_map, (void *)mmt_mem_free, _free_a_map );
	mmt_map_free( events_map, NO );
	mmt_map_free( variables_map, NO );
}

/**
 * Check if code blocks contain a function 'void fn_name'
 * @param fn_name
 * @param code
 * @return
 * TODO: need to exclude the code inside a comment block
 */
static inline bool _hash_function( const char*fn_name, const char *code ){
	if( code == NULL )
		return false;

	while( *code != '\0'){
		//start by "void"
		code = strstr( code, "void" );
		if( code == NULL )
			return false;

		//jump over "void"
		code += 4;

		//jump over space
		while( isspace(*code ) )
			code ++;

		//is end of string?
		if( *code == '\0' )
			return false;

		//compare function name
		if( strncmp( fn_name, code, strlen( fn_name ) ) == 0 )
			return true;
	}
	return false;
}



static void _iterate_variable_if_satisified_to_print( void *key, void *data, void *user_data, size_t index, size_t total ){
	char *str = NULL;
	struct _user_data *_u_data = (struct _user_data *)user_data;
	FILE *fd          = _u_data->file;
	uint32_t rule_id  = _u_data->uint32_val;
	size_t size;
	variable_t *var = (variable_t *) data;

	//init for the first element
	size = expr_stringify_variable( &str, var );
	if( size == 0 ) return;


	//TODO: for now, ignore any variable without ref
	if( var->ref_index == UNKNOWN_REF_INDEX )
		return;

	_gen_code_line( fd );

	fprintf( fd, "\n\n\t data = get_value_from_trace( %"PRIu32", %"PRIu32", %d, trace );",
			var->proto_id, var->att_id, var->ref_index
	);
	_gen_comment_line(fd, "%s.%s.%d", var->proto, var->att, var->ref_index );

	switch( var->data_type ){
	case MMT_SEC_MSG_DATA_TYPE_STRING:
		fprintf( fd, "\n\t const char *%s = (char *) data;", str );
		break;
	case MMT_SEC_MSG_DATA_TYPE_NUMERIC:
		fprintf( fd, "\n\t double %s = 0;", str );
		fprintf( fd, "\n\t if( likely( data != NULL ))  %s = *(double*) data;", str );
		break;
	default:
		fprintf( fd, "\n\t const void *%s = data;", str );
		break;
	}

	mmt_mem_free( str );
}

/**
 * Generate code C for an embedded function in if_satisfied
 * @param fd
 * @param rule
 *
 * Example: if a rule having if_satisfied="update( ngap.ran_ue_id, (ngap.ran_ue_id.1 + 1 + 3))"
 * Then the following code will be generated:
 *
<code>

static void _fn_if_satisified_6 (
       const rule_info_t *rule, int verdict, uint64_t timestamp,
       uint64_t counter, const mmt_array_t * const trace ){

    if( unlikely( trace == NULL )) return;
    const void *data;

    data = get_value_from_trace( 903, 4, 1, trace );
    double _ngap_ran_ue_id_1 = 0;
    if( likely( data != NULL ))  _ngap_ran_ue_id_1 = *(double*) data;

    uint64_t new_val = (_ngap_ran_ue_id_1 + 100);
    mmt_set_attribute_number_value( 903, 4, new_val );

    mmt_forward_packet();
} //end of _fn_if_satisified_6
</code>
 *
 * Finally when the rule is satisfied, MMT will call fn_if_satisified_6
 *
 */

void _gen_if_satisfied_embedded_update_body( FILE *fd, const rule_t *rule, const operation_t *op ){
	char new_fn_name[255];
	const char *fn_string = rule->if_satisfied;
	char *string;
	int i;
	link_node_t *node;

	ASSERT( op->params_size == 2, "Error in if_satisifed function of rule %d: require 2 parameters in 'update' function. "
			"For example: #update( ngap.ran_ue_id, (ngap.ran_ue_id.1 + 3))", rule->id );
	//first parameter
	expression_t *expr = op->params_list->data;
	ASSERT( expr->type == VARIABLE, "Error in if_satisifed function of rule %d: "
			"first parameter of #update function must be an variable in format proto.att", rule->id );

	variable_t *var = expr->variable;
	//no ref_index: we need proto.att, not proto.att.ref
	ASSERT( var->ref_index == UNKNOWN_REF_INDEX, "Error in if_satisifed function of rule %d: "
			"First parameter of #update function must be in format proto.att, not proto.att.ref", rule->id);

	//TODO: support numeric value for now
	ASSERT( var->data_type == MMT_SEC_MSG_DATA_TYPE_NUMERIC,
			"Error in if_satisifed function of rule %d: "
			"Support only attributes having numeric values for now.", rule->id );

	_gen_code_line( fd );
	//as we ensure there are only 2 parameters
	ASSERT( op->params_list->next != NULL,  "Error in if_satisifed function of rule %d: require 2 parameters in 'update' function. "
			"For example: #update( ngap.ran_ue_id, (ngap.ran_ue_id.1 + 3))", rule->id );

	expr = op->params_list->next->data;
	//generate variables
	mmt_map_t *map;
	//set of variables
	size_t var_count = get_unique_variables_of_expression( expr->father, &map, YES );
	if( var_count != 0 ){
			fprintf(fd, "\n\t if( unlikely( trace == NULL )) return;" );
			fprintf(fd, "\n\t const void *data;\n");
			struct _user_data user_data = {.file = fd};
			mmt_map_iterate( map, _iterate_variable_if_satisified_to_print, &user_data );
	}
	mmt_map_free(map, NO);

	expr_stringify_expression(&string, expr );
	fprintf( fd, "\n\n\t uint64_t new_val = %s;", string);
	mmt_mem_free( string );

	fprintf( fd, "\n\t mmt_set_attribute_number_value( %"PRIu32", %"PRIu32", new_val );",
			var->proto_id, var->att_id);
	_gen_comment_line( fd, "\n\t modify %s.%s", var->proto, var->att );

	//explicitly require to forward packet
	fprintf( fd, "\n\n\t mmt_forward_packet();" );

	//here we call set_number_update in pre_embedded_functions
	//fprintf( fd, "\n\n\t set_number_%s;", string ); //change the function name to
	mmt_mem_free( string );
}

void _gen_if_satisfied_embedded_drop_body( FILE *fd, const rule_t *rule, const operation_t *op ){
	ASSERT( op->params_size == 0, "Error in if_satisifed function of rule %d: Do not require any parameter in 'drop' function.",
			rule->id );
	//we simply call this function that is implemented in ../forward/forward_packet.c
	fprintf( fd, "\n\n\t mmt_do_not_forward_packet();" );
}

/**
 * Generate body function for #fuzz
 * @param fd
 * @param rule
 * @param op

For example, if we have if_satisfied="#fuzz(ngap.ran_ue_id, ngap.amf_ue_id, ngap.procedure_code, ngap.pdu_present )"
The following code will be generated:

<code>
 static void _fn_if_satisified_7 (
       const rule_info_t *rule, int verdict, uint64_t timestamp,
       uint64_t counter, const mmt_array_t * const trace ){

    srandom(time(NULL));//init the seed
    mmt_set_attribute_number_value( 903, 4, random() );
    mmt_forward_packet();
    mmt_set_attribute_number_value( 903, 3, random() );
    mmt_forward_packet();
    mmt_set_attribute_number_value( 903, 1, random() );
    mmt_forward_packet();
    mmt_set_attribute_number_value( 903, 2, random() );
    mmt_forward_packet();
} //end of _fn_if_satisified_7
</code>
 */
void _gen_if_satisfied_embedded_fuzz_body( FILE *fd, const rule_t *rule, const operation_t *op ){
	char *string;
	ASSERT( op->params_size > 0, "Error in if_satisifed function of rule %d: Require at least one parameter in 'fuzz' function.",
					rule->id );
	link_node_t *node = op->params_list;
	int i = 0;
	//fprintf(fd, "\n\t srandom(time(NULL));//init the seed ");
	//for each proto.att to be fuzzed
	while( node ){
		i ++;
		expression_t *expr = node->data;
		ASSERT( expr->type == VARIABLE, "Error in if_satisifed function of rule %d: "
				"%d-th parameter of #fuzz function must be an variable in format proto.att", rule->id, i );

		variable_t *var = expr->variable;
		//no ref_index: we need proto.att, not proto.att.ref
		ASSERT( var->ref_index == UNKNOWN_REF_INDEX, "Error in if_satisifed function of rule %d: "
				"%d-th parameter of #fuzz function must be in format proto.att, not proto.att.ref", rule->id, i);

		//TODO: support numeric value for now
		ASSERT( var->data_type == MMT_SEC_MSG_DATA_TYPE_NUMERIC,
				"Error in if_satisifed function of rule %d: "
				"%d-th parameter supports only numeric values for now.", rule->id, i );

		fprintf( fd, "\n\t mmt_set_attribute_number_value( %"PRIu32", %"PRIu32", random() );",
				var->proto_id, var->att_id);
		_gen_comment_line( fd, "\n\t modify %s.%s", var->proto, var->att );
		//forward packet immediately
		fprintf( fd, "\n\t mmt_forward_packet();" );

		node = node->next;
	}
}

void _gen_if_satisfied_embedded_function_for_a_rule( FILE *fd, const rule_t *rule ){
	char new_fn_name[255];
	const char *fn_string = rule->if_satisfied;
	char *string;
	int i;
	link_node_t *node;

	_get_if_statisfied_function_name( rule, new_fn_name, sizeof( new_fn_name));

	fprintf(fd, "\n");

	_gen_comment( fd, "Rule %d - Generated embedded function: %s", rule->id, fn_string );
	fprintf(fd, "static void %s (\n"
			"\t\t const rule_info_t *rule, int verdict, uint64_t timestamp, \n"
			"\t\t uint64_t counter, const mmt_array_t * const trace ){\n", new_fn_name );
	//fprintf(fd, )
	expression_t *fn_expr = NULL;
	parse_expression( &fn_expr, fn_string, strlen(fn_string) );

	ASSERT( fn_expr->type == OPERATION && fn_expr->operation->params_size == 1,
			"Error: expect an embedded function in if_satisifed of rule %d", rule->id );

	//fn_expr is in format (function)
	const expression_t *expr = fn_expr->operation->params_list->data;
	ASSERT( expr->type == OPERATION && expr->operation->operator == FUNCTION,
			"Error: expect an embedded function in if_satisifed of rule %d", rule->id );

	const operation_t *op = expr->operation;
	//generate body for each kind of function
	if( strcmp( op->name, "update" ) == 0 )
		_gen_if_satisfied_embedded_update_body( fd, rule, op );
	else if( strcmp( op->name, "drop") == 0 )
		_gen_if_satisfied_embedded_drop_body( fd, rule, op );
	else if(strcmp( op->name, "fuzz") == 0 )
		_gen_if_satisfied_embedded_fuzz_body( fd, rule, op );
	else
		ABORT("Error in if_satisifed function of rule %d: Do not support yet '%s' function.", rule->id, op->name);

	fprintf(fd, "\n} //end of %s\n", new_fn_name); //close
	expr_free_an_expression(fn_expr, YES);
}

/**
 * Declare extern functions that must be implemeted in mmt-probe
 * @param fd
 */

static void _gen_static_functions_to_forward_packets( FILE *fd ){
}

/**
 * Public API
 */
int generate_fsm( const char* file_name, rule_t *const* rules, size_t count, const char*embedded_functions ){
	char *str_ptr = NULL;
	size_t i;
	//open file for writing
	FILE *fd = fopen(file_name, "w");
	ASSERT (fd != NULL, "Error 11a: Cannot open file %s for writing", file_name );

	str_ptr = get_current_date_time_string( "%Y-%m-%d %H:%M:%S" );
	_gen_comment( fd, "This file is generated automatically on %s", str_ptr == NULL ? "unknown" : str_ptr );
	mmt_mem_free( str_ptr );

	//include
	fprintf( fd, "#include <string.h>\n#include <stdio.h>\n#include <stdlib.h>\n#include \"plugin_header.h\"\n#include \"mmt_fsm.h\"\n#include \"mmt_lib.h\"\n#include \"pre_embedded_functions.h\"\n");

	//this part allows user add a suffix when compile rule.
	//This will be helpful when linking statically rules into a program.
	// Since it will create different name of public functions, depending on the RULE_SUFFIX.
	//By default, 2 public functions are: mmt_sec_get_rule_version_info, mmt_sec_get_plugin_info
	//When one wants to load at least 2 rule .so files, the functions will create a conflict of the same name.

	fprintf( fd, "\n#ifndef RULE_SUFFIX" );
	fprintf( fd, "\n#define RULE_SUFFIX" );
	fprintf( fd, "\n#endif");

	fprintf( fd, "\n#define __NAME(x,y)    x ## y");
	fprintf( fd, "\n#define  _NAME(x,y)  __NAME(x,y)");
	fprintf( fd, "\n#define   NAME(x)     _NAME(x,RULE_SUFFIX)");

	//4 specific functions (2 defined by users in embedded_functions tag, 2 generated auto)
	fprintf( fd, "\n#define on_load                       NAME(on_load)");
	fprintf( fd, "\n#define on_unload                     NAME(on_unload)");
	fprintf( fd, "\n#define mmt_sec_get_plugin_info       NAME(mmt_sec_get_plugin_info)");
	fprintf( fd, "\n#define mmt_sec_get_rule_version_info NAME(mmt_sec_get_rule_version_info)");

	//embedded_functions
	_gen_comment(fd, "Embedded functions");
	if(embedded_functions != NULL )
		fprintf( fd, "%s", embedded_functions );

	//check if embedded_functions have on_load/on_unload function
	if( !_hash_function("on_load", embedded_functions )){
		_gen_comment_line( fd, "Create a dummy on_load function as it has not been defined by users in embedded_functions tag" );
		fprintf(fd, "\nvoid on_load(){}\n");
		DEBUG("Create a dummy on_load function");
	}
	if( !_hash_function("on_unload", embedded_functions )){
		_gen_comment_line( fd, "Create a dummy on_unload function as it has not been defined by users in embedded_functions tag" );
		fprintf(fd, "\nvoid on_unload(){}\n");
		DEBUG("Create a dummy on_unload function");
	}

	for( i=0; i<count; i++ )
		_gen_fsm_for_a_rule( fd, rules[i] );

	_gen_static_functions_to_forward_packets(fd);
	//embedded functions for if_satisfied
	for( i=0; i<count; i++ )
		if( _is_embedded_function_name( rules[i]->if_satisfied) )
			_gen_if_satisfied_embedded_function_for_a_rule( fd, rules[i] );

	//information of rules
	_gen_rule_information( fd, rules, count );

	fclose(fd);

	return 0;
}

/**
 * Compile the generated code
 */
int compile_gen_code( const char *lib_file, const char *code_file, const char *incl_dir ){
	char cmd_str[ 10000 ];

	sprintf( cmd_str, "/usr/bin/gcc %s -fPIC -shared  %s -o %s -I %s",
			//add debug flag if need
#ifdef DEBUG_MODE
			"-g -O0 -DDEBUG_MODE"
#else
	#ifdef __arm__
				"-O0"
	#else
				"-O3"
	#endif
#endif
			, code_file, lib_file, incl_dir );

	DEBUG("Compile rules: %s", cmd_str );
	return system ( cmd_str );
}
