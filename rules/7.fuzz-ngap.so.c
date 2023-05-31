
/** 1207
 * This file is generated automatically on 2023-05-31 11:07:52
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

//initialize random seed when loading this rule file
void on_load(){
	srand(time(NULL)); 
}

//customized function to
// generate randomly number from 15 -> 20
uint64_t em_my_random(){
	uint64_t max = 20, min = 15;
	return (rand() % (max-min) + min);
}

//override the random function
#define random em_my_random
/** 1245 Create a dummy on_unload function as it has not been defined by users in embedded_functions tag*/
void on_unload(){}


 //======================================RULE 7======================================
 #define EVENTS_COUNT_7 2

 #define PROTO_ATTS_COUNT_7 1

/** 862
 * Proto_atts for rule 7
 */

 static proto_attribute_t proto_atts_7[ PROTO_ATTS_COUNT_7 ] = {{.proto = "ngap", .proto_id = 903, .att = "procedure_code", .att_id = 1, .data_type = 0, .dpi_type = 2}};
/** 874
 * Detail of proto_atts for each event
 */

 static mmt_array_t proto_atts_events_7[ 3 ] = { {.elements_count = 0, .data = NULL}, 
	 {//event_1
		 .elements_count = 1,
		 .data = (void* []) { &proto_atts_7[ 0 ] }
	 },
	 {//event_2
		 .elements_count = 0,
		 .data = NULL
	 } 
 };//end proto_atts_events_

 static mmt_array_t excluded_filter_7[ 3 ] = { {.elements_count = 0, .data = NULL}, 
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
typedef struct _msg_struct_7{
	 uint16_t _ngap_procedure_code;
 }_msg_t_7;
/** 551
 * Create an instance of _msg_t_7
 */
static _msg_t_7 _m7;
 static void _allocate_msg_t_7( const char* proto, const char* att, uint16_t index ){
	 if( strcmp( proto, "ngap" ) == 0 && strcmp( att, "procedure_code" ) == 0 ){ _m7._ngap_procedure_code = index; return; }
 }
/** 92
 * Rule 7, event 1
  * Whether existing NGAP
 */
static inline int g_7_1( const message_t *msg, const fsm_t *fsm ){
	 if( unlikely( msg == NULL || fsm == NULL )) return 0;
	 const message_t *his_msg;
	 const void *data;/* 57 */

	 data = get_element_data_message_t( msg, _m7._ngap_procedure_code );
	 double _ngap_procedure_code = 0;
	 if (data != NULL)  _ngap_procedure_code = *(double*) data;

	 return (_ngap_procedure_code != 0);
 }
 
/** 92
 * Rule 7, event 2
  * 
 */
static inline int g_7_2( const message_t *msg, const fsm_t *fsm ){

	 return (1);
 }
 
/** 410
 * States of FSM for rule 7
 */

/** 411
 * Predefine list of states: init, fail, pass, ...
 */
static fsm_state_t s_7_0, s_7_1, s_7_2, s_7_3, s_7_4;
/** 424
 * Initialize states: init, error, final, ...
 */
static fsm_state_t
/** 430
 * initial state
 */
 s_7_0 = {
	 .delay        = {.time_min = 0, .time_max = 0, .counter_min = 0, .counter_max = 0},
	 .is_temporary = 0,
	 .description  = "Fuzz attributes of NGAP protocol",
	 .entry_action = 0, //FSM_ACTION_DO_NOTHING
	 .exit_action  = 1, //FSM_ACTION_CREATE_INSTANCE
	 .transitions  = (fsm_transition_t[]){
		 /** 458 Whether existing NGAP */
		 /** 460 A real event */
		 { .event_type = 1, .guard = &g_7_1, .action = 1, .target_state = &s_7_4}  //FSM_ACTION_CREATE_INSTANCE
	 },
	 .transitions_count = 1
 },
/** 430
 * timeout/error state
 */
 s_7_1 = {
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
 s_7_2 = {
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
 s_7_3 = {
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
 s_7_4 = {
	 .delay        = {.time_min = 0LL, .time_max = 0LL, .counter_min = 0LL, .counter_max = 0LL},
	 .is_temporary = 1,
	 .description  = "Fuzz attributes of NGAP protocol",
	 .entry_action = 0, //FSM_ACTION_DO_NOTHING
	 .exit_action  = 0, //FSM_ACTION_DO_NOTHING
	 .transitions  = (fsm_transition_t[]){
		 /** 460 Timeout event will fire this transition */
		 { .event_type = 0, .guard = NULL  , .action = 0, .target_state = &s_7_1}, //FSM_ACTION_DO_NOTHING
		 /** 460 A real event */
		 { .event_type = 2, .guard = &g_7_2, .action = 2, .target_state = &s_7_2}  //FSM_ACTION_RESET_TIMER
	 },
	 .transitions_count = 2
 };
/** 487
 * Create a new FSM for this rule
 */
static void *create_new_fsm_7(){
		 return fsm_init( &s_7_0, &s_7_1, &s_7_2, &s_7_3, EVENTS_COUNT_7, sizeof( _msg_t_7 ) );//init, error, final, inconclusive, events_count
 }//end function

/** 1157
 * Rule 7 - Generated embedded function: #fuzz(ngap.ran_ue_id, ngap.amf_ue_id, ngap.procedure_code, ngap.pdu_present )
 */
static void _fn_if_satisified_7 (
		 const rule_info_t *rule, int verdict, uint64_t timestamp, 
		 uint64_t counter, const mmt_array_t * const trace ){

	 mmt_set_attribute_number_value( 903, 4, random() );/** 1138 
	 modify ngap.ran_ue_id*/
	 mmt_forward_packet();
	 mmt_set_attribute_number_value( 903, 3, random() );/** 1138 
	 modify ngap.amf_ue_id*/
	 mmt_forward_packet();
	 mmt_set_attribute_number_value( 903, 1, random() );/** 1138 
	 modify ngap.procedure_code*/
	 mmt_forward_packet();
	 mmt_set_attribute_number_value( 903, 2, random() );/** 1138 
	 modify ngap.pdu_present*/
	 mmt_forward_packet();
} //end of _fn_if_satisified_7

/** 609
 * Moment the rules being encoded
  * PUBLIC API
 */

static const rule_version_info_t version = {.created_date=1685524072, .hash = "dcce935", .number="0.0.6", .index=600, .dpi="1.7.8 (68bd7d93)"};
const rule_version_info_t * mmt_sec_get_rule_version_info(){ return &version;};

 //======================================GENERAL======================================
/** 623
 * Information of 1 rules
  * PUBLIC API
 */
size_t mmt_sec_get_plugin_info( const rule_info_t **rules_arr ){
	  static const rule_info_t rules[] = (rule_info_t[]){
		 {
			 .id               = 7,
			 .type_id          = 4,
			 .type_string      = "FORWARD",
			 .events_count     = EVENTS_COUNT_7,
			 .description      = "Fuzz attributes of NGAP protocol",
			 .if_satisfied     = _fn_if_satisified_7,
			 .if_not_satisfied = NULL,
			 .proto_atts_count = PROTO_ATTS_COUNT_7,
			 .proto_atts       = proto_atts_7,
			 .proto_atts_events= proto_atts_events_7,
			 .excluded_filter  = excluded_filter_7,
			 .create_instance  = &create_new_fsm_7,
			 .hash_message     = &_allocate_msg_t_7,
			 .version          = &version,
		 }
	 };
	 *rules_arr = rules;
	 return 1;
 }