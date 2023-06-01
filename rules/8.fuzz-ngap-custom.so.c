
/** 1207
 * This file is generated automatically on 2023-06-01 11:11:44
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
// see https://linux.die.net/man/3/rand
void on_load(){
	srand(time(NULL)); 
}

//customized function to
// generate randomly number in [min, max]
static inline int em_rand(int max, int min){
	return (rand() % (max-min) + min);
}

static inline void em_fuzz( int proto_id, int att_id, int min, int max){
	int val = em_rand(min, max);
	printf("Fuzz att %d of proto %d with %d\n", proto_id, att_id, val);
	set_numeric_value( proto_id, att_id, val );
	forward_packet(); 
}


//callback function when its rule is satisfied
static void em_custom_fuzzer( const rule_info_t *rule, int verdict, uint64_t timestamp, 
		uint64_t counter, const mmt_array_t * const trace ){
	bool ret = true;
	//sent the original packet
	forward_packet();

	//see the constants here:
	// https://github.com/Montimage/mmt-dpi/blob/master/src/mmt_mobile/include/proto_ngap.h
	
	//fuzz ran_ue_id
	em_fuzz( PROTO_NGAP, NGAP_ATT_RAN_UE_ID, 100, 110 );
	
	//fuzz amf_ue_id
	em_fuzz( PROTO_NGAP, NGAP_ATT_AMF_UE_ID, 110, 120 );
	
	//fuzz procedure code
	em_fuzz( PROTO_NGAP, NGAP_ATT_PROCEDURE_CODE, 1, 10 );
}
/** 1245 Create a dummy on_unload function as it has not been defined by users in embedded_functions tag*/
void on_unload(){}


 //======================================RULE 8======================================
 #define EVENTS_COUNT_8 2

 #define PROTO_ATTS_COUNT_8 1

/** 862
 * Proto_atts for rule 8
 */

 static proto_attribute_t proto_atts_8[ PROTO_ATTS_COUNT_8 ] = {{.proto = "ngap", .proto_id = 903, .att = "procedure_code", .att_id = 1, .data_type = 0, .dpi_type = 2}};
/** 874
 * Detail of proto_atts for each event
 */

 static mmt_array_t proto_atts_events_8[ 3 ] = { {.elements_count = 0, .data = NULL}, 
	 {//event_1
		 .elements_count = 1,
		 .data = (void* []) { &proto_atts_8[ 0 ] }
	 },
	 {//event_2
		 .elements_count = 0,
		 .data = NULL
	 } 
 };//end proto_atts_events_

 static mmt_array_t excluded_filter_8[ 3 ] = { {.elements_count = 0, .data = NULL}, 
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
typedef struct _msg_struct_8{
	 uint16_t _ngap_procedure_code;
 }_msg_t_8;
/** 551
 * Create an instance of _msg_t_8
 */
static _msg_t_8 _m8;
 static void _allocate_msg_t_8( const char* proto, const char* att, uint16_t index ){
	 if( strcmp( proto, "ngap" ) == 0 && strcmp( att, "procedure_code" ) == 0 ){ _m8._ngap_procedure_code = index; return; }
 }
/** 92
 * Rule 8, event 1
  * Whether existing NGAP
 */
static inline int g_8_1( const message_t *msg, const fsm_t *fsm ){
	 if( unlikely( msg == NULL || fsm == NULL )) return 0;
	 const message_t *his_msg;
	 const void *data;/* 57 */

	 data = get_element_data_message_t( msg, _m8._ngap_procedure_code );
	 double _ngap_procedure_code = 0;
	 if (data != NULL)  _ngap_procedure_code = *(double*) data;

	 return (_ngap_procedure_code != 0);
 }
 
/** 92
 * Rule 8, event 2
  * 
 */
static inline int g_8_2( const message_t *msg, const fsm_t *fsm ){

	 return (1);
 }
 
/** 410
 * States of FSM for rule 8
 */

/** 411
 * Predefine list of states: init, fail, pass, ...
 */
static fsm_state_t s_8_0, s_8_1, s_8_2, s_8_3, s_8_4;
/** 424
 * Initialize states: init, error, final, ...
 */
static fsm_state_t
/** 430
 * initial state
 */
 s_8_0 = {
	 .delay        = {.time_min = 0, .time_max = 0, .counter_min = 0, .counter_max = 0},
	 .is_temporary = 0,
	 .description  = "Fuzz attributes of NGAP protocol",
	 .entry_action = 0, //FSM_ACTION_DO_NOTHING
	 .exit_action  = 1, //FSM_ACTION_CREATE_INSTANCE
	 .transitions  = (fsm_transition_t[]){
		 /** 458 Whether existing NGAP */
		 /** 460 A real event */
		 { .event_type = 1, .guard = &g_8_1, .action = 1, .target_state = &s_8_4}  //FSM_ACTION_CREATE_INSTANCE
	 },
	 .transitions_count = 1
 },
/** 430
 * timeout/error state
 */
 s_8_1 = {
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
 s_8_2 = {
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
 s_8_3 = {
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
 s_8_4 = {
	 .delay        = {.time_min = 0LL, .time_max = 0LL, .counter_min = 0LL, .counter_max = 0LL},
	 .is_temporary = 1,
	 .description  = "Fuzz attributes of NGAP protocol",
	 .entry_action = 0, //FSM_ACTION_DO_NOTHING
	 .exit_action  = 0, //FSM_ACTION_DO_NOTHING
	 .transitions  = (fsm_transition_t[]){
		 /** 460 Timeout event will fire this transition */
		 { .event_type = 0, .guard = NULL  , .action = 0, .target_state = &s_8_1}, //FSM_ACTION_DO_NOTHING
		 /** 460 A real event */
		 { .event_type = 2, .guard = &g_8_2, .action = 2, .target_state = &s_8_2}  //FSM_ACTION_RESET_TIMER
	 },
	 .transitions_count = 2
 };
/** 487
 * Create a new FSM for this rule
 */
static void *create_new_fsm_8(){
		 return fsm_init( &s_8_0, &s_8_1, &s_8_2, &s_8_3, EVENTS_COUNT_8, sizeof( _msg_t_8 ) );//init, error, final, inconclusive, events_count
 }//end function
/** 609
 * Moment the rules being encoded
  * PUBLIC API
 */

static const rule_version_info_t version = {.created_date=1685610704, .hash = "6d0f9d6", .number="0.0.6", .index=600, .dpi="1.7.8 (68bd7d93)"};
const rule_version_info_t * mmt_sec_get_rule_version_info(){ return &version;};

 //======================================GENERAL======================================
/** 623
 * Information of 1 rules
  * PUBLIC API
 */
size_t mmt_sec_get_plugin_info( const rule_info_t **rules_arr ){
	  static const rule_info_t rules[] = (rule_info_t[]){
		 {
			 .id               = 8,
			 .type_id          = 4,
			 .type_string      = "FORWARD",
			 .events_count     = EVENTS_COUNT_8,
			 .description      = "Fuzz attributes of NGAP protocol",
			 .if_satisfied     = em_custom_fuzzer,
			 .if_not_satisfied = NULL,
			 .proto_atts_count = PROTO_ATTS_COUNT_8,
			 .proto_atts       = proto_atts_8,
			 .proto_atts_events= proto_atts_events_8,
			 .excluded_filter  = excluded_filter_8,
			 .create_instance  = &create_new_fsm_8,
			 .hash_message     = &_allocate_msg_t_8,
			 .version          = &version,
		 }
	 };
	 *rules_arr = rules;
	 return 1;
 }