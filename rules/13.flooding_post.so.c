
/** 1207
 * This file is generated automatically on 2023-06-01 11:11:40
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "plugin_header.h"
#include "mmt_fsm.h"
#include "mmt_lib.h"
#include "pre_embedded_functions.h"

#ifndef RULE_SUFFIX
#define RULE_SUFFIX
#endif
#define __NAME(x,y)    x ## y
#define  _NAME(x,y)  __NAME(x,y)
#define   NAME(x)     _NAME(x,RULE_SUFFIX)
#define on_load                       NAME(on_load)
#define on_unload                     NAME(on_unload)
#define mmt_sec_get_plugin_info       NAME(mmt_sec_get_plugin_info)
#define mmt_sec_get_rule_version_info NAME(mmt_sec_get_rule_version_info)
/** 1234
 * Embedded functions
 */

#include "../lib/http2.h"
#include <stdlib.h>

static unsigned long int number_request = 0;



	void on_load(){
		const char *str = getenv("REQ_NUMBER");
		if( str == NULL ){
			
			//printf("Rule 13: no value of REQ_NUMBER\n" );
			number_request=50000;
		}
		else{
		number_request = strtoul( str, NULL, 0 );
		}
		//printf("NUMBER_REQUESTS %lu\n", number_request);
		//printf("Loaded successfully rule http2 Post  \n" );
		
	}
	
	static void em_modif_then_forward(
	   const rule_info_t *rule, int verdict, uint64_t timestamp, 
	   uint64_t counter, const mmt_array_t * const trace ){	
  	const char* trace_str = mmt_convert_execution_trace_to_json_string( trace, rule );
   	//printf( "DETECT Header Method Post %d\n%s\n", rule->id, trace_str );
   	// uint32_t stream_id = get_numeric_value( PROTO_HTTP2, HTTP2_STREAM_ID, 2, trace );
   	//printf( "em_modif_then_forward \n");
   	//for(int i=0;i<20;i++){


	for(int i=1;i<number_request;i+=2){
		
     		set_numeric_value( PROTO_HTTP2, HTTP2_HEADER_STREAM_ID, i );
     		forward_packet();
     	         
     	       }

	}



	void on_unload(){
		//printf("Unloaded successfully rule 1\n");
	}



 //======================================RULE 13======================================
 #define EVENTS_COUNT_13 2

 #define PROTO_ATTS_COUNT_13 1

/** 862
 * Proto_atts for rule 13
 */

 static proto_attribute_t proto_atts_13[ PROTO_ATTS_COUNT_13 ] = {{.proto = "http2", .proto_id = 700, .att = "header_method", .att_id = 2, .data_type = 0, .dpi_type = 1}};
/** 874
 * Detail of proto_atts for each event
 */

 static mmt_array_t proto_atts_events_13[ 3 ] = { {.elements_count = 0, .data = NULL}, 
	 {//event_1
		 .elements_count = 1,
		 .data = (void* []) { &proto_atts_13[ 0 ] }
	 },
	 {//event_2
		 .elements_count = 1,
		 .data = (void* []) { &proto_atts_13[ 0 ] }
	 } 
 };//end proto_atts_events_

 static mmt_array_t excluded_filter_13[ 3 ] = { {.elements_count = 0, .data = NULL}, 
	 {//event_1
		 .elements_count = 0,
		 .data = NULL
	 },
	 {//event_2
		 .elements_count = 0,
		 .data = NULL
	 } 
 };//end excluded_filter_

/** 517
 * Structure to represent event data
 */
typedef struct _msg_struct_13{
	 uint16_t _http2_header_method;
 }_msg_t_13;
/** 551
 * Create an instance of _msg_t_13
 */
static _msg_t_13 _m13;
 static void _allocate_msg_t_13( const char* proto, const char* att, uint16_t index ){
	 if( strcmp( proto, "http2" ) == 0 && strcmp( att, "header_method" ) == 0 ){ _m13._http2_header_method = index; return; }
 }
/** 92
 * Rule 13, event 1
  * Modify stream id
 */
static inline int g_13_1( const message_t *msg, const fsm_t *fsm ){
	 if( unlikely( msg == NULL || fsm == NULL )) return 0;
	 const message_t *his_msg;
	 const void *data;/* 57 */

	 data = get_element_data_message_t( msg, _m13._http2_header_method );
	 double _http2_header_method = 0;
	 if (data != NULL)  _http2_header_method = *(double*) data;

	 return (_http2_header_method == 131);
 }
 
/** 92
 * Rule 13, event 2
  * Nothing
 */
static inline int g_13_2( const message_t *msg, const fsm_t *fsm ){
	 if( unlikely( msg == NULL || fsm == NULL )) return 0;
	 const message_t *his_msg;
	 const void *data;/* 57 */

	 data = get_element_data_message_t( msg, _m13._http2_header_method );
	 double _http2_header_method = 0;
	 if (data != NULL)  _http2_header_method = *(double*) data;

	 return (_http2_header_method != 0);
 }
 
/** 410
 * States of FSM for rule 13
 */

/** 411
 * Predefine list of states: init, fail, pass, ...
 */
static fsm_state_t s_13_0, s_13_1, s_13_2, s_13_3, s_13_4;
/** 424
 * Initialize states: init, error, final, ...
 */
static fsm_state_t
/** 430
 * initial state
 */
 s_13_0 = {
	 .delay        = {.time_min = 0, .time_max = 0, .counter_min = 0, .counter_max = 0},
	 .is_temporary = 0,
	 .description  = "Modify stream id",
	 .entry_action = 0, //FSM_ACTION_DO_NOTHING
	 .exit_action  = 1, //FSM_ACTION_CREATE_INSTANCE
	 .transitions  = (fsm_transition_t[]){
		 /** 458 Modify stream id */
		 /** 460 A real event */
		 { .event_type = 1, .guard = &g_13_1, .action = 1, .target_state = &s_13_4}  //FSM_ACTION_CREATE_INSTANCE
	 },
	 .transitions_count = 1
 },
/** 430
 * timeout/error state
 */
 s_13_1 = {
	 .delay        = {.time_min = 0, .time_max = 0, .counter_min = 0, .counter_max = 0},
	 .is_temporary = 0,
	 .description  =  NULL ,
	 .entry_action = 0, //FSM_ACTION_DO_NOTHING
	 .exit_action  = 0, //FSM_ACTION_DO_NOTHING
	 .transitions  = NULL,
	 .transitions_count = 0
 },
/** 430
 * pass state
 */
 s_13_2 = {
	 .delay        = {.time_min = 0, .time_max = 0, .counter_min = 0, .counter_max = 0},
	 .is_temporary = 0,
	 .description  =  NULL ,
	 .entry_action = 0, //FSM_ACTION_DO_NOTHING
	 .exit_action  = 0, //FSM_ACTION_DO_NOTHING
	 .transitions  = NULL,
	 .transitions_count = 0
 },
/** 430
 * inconclusive state
 */
 s_13_3 = {
	 .delay        = {.time_min = 0, .time_max = 0, .counter_min = 0, .counter_max = 0},
	 .is_temporary = 0,
	 .description  =  NULL ,
	 .entry_action = 0, //FSM_ACTION_DO_NOTHING
	 .exit_action  = 0, //FSM_ACTION_DO_NOTHING
	 .transitions  = NULL,
	 .transitions_count = 0
 },
/** 430
 * root node
 */
 s_13_4 = {
	 .delay        = {.time_min = 0LL, .time_max = 0LL, .counter_min = 0LL, .counter_max = 0LL},
	 .is_temporary = 1,
	 .description  = "Modify stream id",
	 .entry_action = 0, //FSM_ACTION_DO_NOTHING
	 .exit_action  = 0, //FSM_ACTION_DO_NOTHING
	 .transitions  = (fsm_transition_t[]){
		 /** 460 Timeout event will fire this transition */
		 { .event_type = 0, .guard = NULL  , .action = 0, .target_state = &s_13_1}, //FSM_ACTION_DO_NOTHING
		 /** 458 Nothing */
		 /** 460 A real event */
		 { .event_type = 2, .guard = &g_13_2, .action = 2, .target_state = &s_13_2}  //FSM_ACTION_RESET_TIMER
	 },
	 .transitions_count = 2
 };
/** 487
 * Create a new FSM for this rule
 */
static void *create_new_fsm_13(){
		 return fsm_init( &s_13_0, &s_13_1, &s_13_2, &s_13_3, EVENTS_COUNT_13, sizeof( _msg_t_13 ) );//init, error, final, inconclusive, events_count
 }//end function
/** 609
 * Moment the rules being encoded
  * PUBLIC API
 */

static const rule_version_info_t version = {.created_date=1685610700, .hash = "6d0f9d6", .number="0.0.6", .index=600, .dpi="1.7.8 (68bd7d93)"};
const rule_version_info_t * mmt_sec_get_rule_version_info(){ return &version;};

 //======================================GENERAL======================================
/** 623
 * Information of 1 rules
  * PUBLIC API
 */
size_t mmt_sec_get_plugin_info( const rule_info_t **rules_arr ){
	  static const rule_info_t rules[] = (rule_info_t[]){
		 {
			 .id               = 13,
			 .type_id          = 4,
			 .type_string      = "FORWARD",
			 .events_count     = EVENTS_COUNT_13,
			 .description      = "Modify stream id",
			 .if_satisfied     = em_modif_then_forward,
			 .if_not_satisfied = NULL,
			 .proto_atts_count = PROTO_ATTS_COUNT_13,
			 .proto_atts       = proto_atts_13,
			 .proto_atts_events= proto_atts_events_13,
			 .excluded_filter  = excluded_filter_13,
			 .create_instance  = &create_new_fsm_13,
			 .hash_message     = &_allocate_msg_t_13,
			 .version          = &version,
		 }
	 };
	 *rules_arr = rules;
	 return 1;
 }