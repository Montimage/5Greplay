
/** 1207
 * This file is generated automatically on 2023-05-31 11:07:51
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
/** 1240 Create a dummy on_load function as it has not been defined by users in embedded_functions tag*/
void on_load(){}
/** 1245 Create a dummy on_unload function as it has not been defined by users in embedded_functions tag*/
void on_unload(){}


 //======================================RULE 6======================================
 #define EVENTS_COUNT_6 2

 #define PROTO_ATTS_COUNT_6 3

/** 862
 * Proto_atts for rule 6
 */

 static proto_attribute_t proto_atts_6[ PROTO_ATTS_COUNT_6 ] = {{.proto = "nas_5g", .proto_id = 904, .att = "message_type", .att_id = 2, .data_type = 0, .dpi_type = 1},
 {.proto = "ngap", .proto_id = 903, .att = "procedure_code", .att_id = 1, .data_type = 0, .dpi_type = 2},
 {.proto = "ngap", .proto_id = 903, .att = "ran_ue_id", .att_id = 4, .data_type = 0, .dpi_type = 4}};
/** 874
 * Detail of proto_atts for each event
 */

 static mmt_array_t proto_atts_events_6[ 3 ] = { {.elements_count = 0, .data = NULL}, 
	 {//event_1
		 .elements_count = 2,
		 .data = (void* []) { &proto_atts_6[ 0 ] ,  &proto_atts_6[ 1 ] }
	 },
	 {//event_2
		 .elements_count = 1,
		 .data = (void* []) { &proto_atts_6[ 2 ] }
	 } 
 };//end proto_atts_events_

 static mmt_array_t excluded_filter_6[ 3 ] = { {.elements_count = 0, .data = NULL}, 
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
typedef struct _msg_struct_6{
	 uint16_t _nas_5g_message_type;
	 uint16_t _ngap_procedure_code;
	 uint16_t _ngap_ran_ue_id;
 }_msg_t_6;
/** 551
 * Create an instance of _msg_t_6
 */
static _msg_t_6 _m6;
 static void _allocate_msg_t_6( const char* proto, const char* att, uint16_t index ){
	 if( strcmp( proto, "nas_5g" ) == 0 && strcmp( att, "message_type" ) == 0 ){ _m6._nas_5g_message_type = index; return; }
	 if( strcmp( proto, "ngap" ) == 0 && strcmp( att, "procedure_code" ) == 0 ){ _m6._ngap_procedure_code = index; return; }
	 if( strcmp( proto, "ngap" ) == 0 && strcmp( att, "ran_ue_id" ) == 0 ){ _m6._ngap_ran_ue_id = index; return; }
 }
/** 92
 * Rule 6, event 1
  * Downlink NAS Transport, Authentication request
 */
static inline int g_6_1( const message_t *msg, const fsm_t *fsm ){
	 if( unlikely( msg == NULL || fsm == NULL )) return 0;
	 const message_t *his_msg;
	 const void *data;/* 57 */

	 data = get_element_data_message_t( msg, _m6._nas_5g_message_type );
	 double _nas_5g_message_type = 0;
	 if (data != NULL)  _nas_5g_message_type = *(double*) data;/* 57 */

	 data = get_element_data_message_t( msg, _m6._ngap_procedure_code );
	 double _ngap_procedure_code = 0;
	 if (data != NULL)  _ngap_procedure_code = *(double*) data;

	 return ((_ngap_procedure_code == 4) && (_nas_5g_message_type == 86));
 }
 
/** 92
 * Rule 6, event 2
  * Nothing
 */
static inline int g_6_2( const message_t *msg, const fsm_t *fsm ){
	 if( unlikely( msg == NULL || fsm == NULL )) return 0;
	 const message_t *his_msg;
	 const void *data;/* 57 */

	 data = get_element_data_message_t( msg, _m6._ngap_ran_ue_id );
	 double _ngap_ran_ue_id = 0;
	 if (data != NULL)  _ngap_ran_ue_id = *(double*) data;

	 return (_ngap_ran_ue_id != 0);
 }
 
/** 410
 * States of FSM for rule 6
 */

/** 411
 * Predefine list of states: init, fail, pass, ...
 */
static fsm_state_t s_6_0, s_6_1, s_6_2, s_6_3, s_6_4;
/** 424
 * Initialize states: init, error, final, ...
 */
static fsm_state_t
/** 430
 * initial state
 */
 s_6_0 = {
	 .delay        = {.time_min = 0, .time_max = 0, .counter_min = 0, .counter_max = 0},
	 .is_temporary = 0,
	 .description  = "5G testing",
	 .entry_action = 0, //FSM_ACTION_DO_NOTHING
	 .exit_action  = 1, //FSM_ACTION_CREATE_INSTANCE
	 .transitions  = (fsm_transition_t[]){
		 /** 458 Downlink NAS Transport, Authentication request */
		 /** 460 A real event */
		 { .event_type = 1, .guard = &g_6_1, .action = 1, .target_state = &s_6_4}  //FSM_ACTION_CREATE_INSTANCE
	 },
	 .transitions_count = 1
 },
/** 430
 * timeout/error state
 */
 s_6_1 = {
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
 s_6_2 = {
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
 s_6_3 = {
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
 s_6_4 = {
	 .delay        = {.time_min = 0LL, .time_max = 0LL, .counter_min = 0LL, .counter_max = 0LL},
	 .is_temporary = 1,
	 .description  = "5G testing",
	 .entry_action = 0, //FSM_ACTION_DO_NOTHING
	 .exit_action  = 0, //FSM_ACTION_DO_NOTHING
	 .transitions  = (fsm_transition_t[]){
		 /** 460 Timeout event will fire this transition */
		 { .event_type = 0, .guard = NULL  , .action = 0, .target_state = &s_6_1}, //FSM_ACTION_DO_NOTHING
		 /** 458 Nothing */
		 /** 460 A real event */
		 { .event_type = 2, .guard = &g_6_2, .action = 2, .target_state = &s_6_2}  //FSM_ACTION_RESET_TIMER
	 },
	 .transitions_count = 2
 };
/** 487
 * Create a new FSM for this rule
 */
static void *create_new_fsm_6(){
		 return fsm_init( &s_6_0, &s_6_1, &s_6_2, &s_6_3, EVENTS_COUNT_6, sizeof( _msg_t_6 ) );//init, error, final, inconclusive, events_count
 }//end function

/** 1157
 * Rule 6 - Generated embedded function: #update(ngap.ran_ue_id, (ngap.ran_ue_id.1 + 100))
 */
static void _fn_if_satisified_6 (
		 const rule_info_t *rule, int verdict, uint64_t timestamp, 
		 uint64_t counter, const mmt_array_t * const trace ){
/* 1045 */
	 if( unlikely( trace == NULL )) return;
	 const void *data;
/* 966 */

	 data = get_value_from_trace( 903, 4, 1, trace );/** 971 ngap.ran_ue_id.1*/
	 double _ngap_ran_ue_id_1 = 0;
	 if( likely( data != NULL ))  _ngap_ran_ue_id_1 = *(double*) data;

	 uint64_t new_val = (_ngap_ran_ue_id_1 + 100);
	 mmt_set_attribute_number_value( 903, 4, new_val );/** 1069 
	 modify ngap.ran_ue_id*/

	 mmt_forward_packet();
} //end of _fn_if_satisified_6

/** 609
 * Moment the rules being encoded
  * PUBLIC API
 */

static const rule_version_info_t version = {.created_date=1685524071, .hash = "dcce935", .number="0.0.6", .index=600, .dpi="1.7.8 (68bd7d93)"};
const rule_version_info_t * mmt_sec_get_rule_version_info(){ return &version;};

 //======================================GENERAL======================================
/** 623
 * Information of 1 rules
  * PUBLIC API
 */
size_t mmt_sec_get_plugin_info( const rule_info_t **rules_arr ){
	  static const rule_info_t rules[] = (rule_info_t[]){
		 {
			 .id               = 6,
			 .type_id          = 4,
			 .type_string      = "FORWARD",
			 .events_count     = EVENTS_COUNT_6,
			 .description      = "5G testing",
			 .if_satisfied     = _fn_if_satisified_6,
			 .if_not_satisfied = NULL,
			 .proto_atts_count = PROTO_ATTS_COUNT_6,
			 .proto_atts       = proto_atts_6,
			 .proto_atts_events= proto_atts_events_6,
			 .excluded_filter  = excluded_filter_6,
			 .create_instance  = &create_new_fsm_6,
			 .hash_message     = &_allocate_msg_t_6,
			 .version          = &version,
		 }
	 };
	 *rules_arr = rules;
	 return 1;
 }