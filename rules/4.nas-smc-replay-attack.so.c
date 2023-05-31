
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


 //======================================RULE 4======================================
 #define EVENTS_COUNT_4 2

 #define PROTO_ATTS_COUNT_4 2

/** 862
 * Proto_atts for rule 4
 */

 static proto_attribute_t proto_atts_4[ PROTO_ATTS_COUNT_4 ] = {{.proto = "nas_5g", .proto_id = 904, .att = "message_type", .att_id = 2, .data_type = 0, .dpi_type = 1},
 {.proto = "nas_5g", .proto_id = 904, .att = "security_type", .att_id = 3, .data_type = 0, .dpi_type = 1}};
/** 874
 * Detail of proto_atts for each event
 */

 static mmt_array_t proto_atts_events_4[ 3 ] = { {.elements_count = 0, .data = NULL}, 
	 {//event_1
		 .elements_count = 1,
		 .data = (void* []) { &proto_atts_4[ 0 ] }
	 },
	 {//event_2
		 .elements_count = 1,
		 .data = (void* []) { &proto_atts_4[ 1 ] }
	 } 
 };//end proto_atts_events_

 static mmt_array_t excluded_filter_4[ 3 ] = { {.elements_count = 0, .data = NULL}, 
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
typedef struct _msg_struct_4{
	 uint16_t _nas_5g_message_type;
	 uint16_t _nas_5g_security_type;
 }_msg_t_4;
/** 551
 * Create an instance of _msg_t_4
 */
static _msg_t_4 _m4;
 static void _allocate_msg_t_4( const char* proto, const char* att, uint16_t index ){
	 if( strcmp( proto, "nas_5g" ) == 0 && strcmp( att, "message_type" ) == 0 ){ _m4._nas_5g_message_type = index; return; }
	 if( strcmp( proto, "nas_5g" ) == 0 && strcmp( att, "security_type" ) == 0 ){ _m4._nas_5g_security_type = index; return; }
 }
/** 92
 * Rule 4, event 1
  * NAS Security mode COMMAND
 */
static inline int g_4_1( const message_t *msg, const fsm_t *fsm ){
	 if( unlikely( msg == NULL || fsm == NULL )) return 0;
	 const message_t *his_msg;
	 const void *data;/* 57 */

	 data = get_element_data_message_t( msg, _m4._nas_5g_message_type );
	 double _nas_5g_message_type = 0;
	 if (data != NULL)  _nas_5g_message_type = *(double*) data;

	 return (_nas_5g_message_type == 93);
 }
 
/** 92
 * Rule 4, event 2
  * NAS Security mode COMPLETE
 */
static inline int g_4_2( const message_t *msg, const fsm_t *fsm ){
	 if( unlikely( msg == NULL || fsm == NULL )) return 0;
	 const message_t *his_msg;
	 const void *data;/* 57 */

	 data = get_element_data_message_t( msg, _m4._nas_5g_security_type );
	 double _nas_5g_security_type = 0;
	 if (data != NULL)  _nas_5g_security_type = *(double*) data;

	 return (_nas_5g_security_type == 4);
 }
 
/** 410
 * States of FSM for rule 4
 */

/** 411
 * Predefine list of states: init, fail, pass, ...
 */
static fsm_state_t s_4_0, s_4_1, s_4_2, s_4_3, s_4_4;
/** 424
 * Initialize states: init, error, final, ...
 */
static fsm_state_t
/** 430
 * initial state
 */
 s_4_0 = {
	 .delay        = {.time_min = 0, .time_max = 0, .counter_min = 0, .counter_max = 0},
	 .is_temporary = 0,
	 .description  = "Forwarding NAS security mode COMPLETE that answers to NAS security mode COMMAND ",
	 .entry_action = 0, //FSM_ACTION_DO_NOTHING
	 .exit_action  = 1, //FSM_ACTION_CREATE_INSTANCE
	 .transitions  = (fsm_transition_t[]){
		 /** 458 NAS Security mode COMMAND */
		 /** 460 A real event */
		 { .event_type = 1, .guard = &g_4_1, .action = 1, .target_state = &s_4_4}  //FSM_ACTION_CREATE_INSTANCE
	 },
	 .transitions_count = 1
 },
/** 430
 * timeout/error state
 */
 s_4_1 = {
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
 s_4_2 = {
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
 s_4_3 = {
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
 s_4_4 = {
	 .delay        = {.time_min = 0LL, .time_max = 10000LL, .counter_min = 0LL, .counter_max = 0LL},
	 .is_temporary = 0,
	 .description  = "Forwarding NAS security mode COMPLETE that answers to NAS security mode COMMAND ",
	 .entry_action = 0, //FSM_ACTION_DO_NOTHING
	 .exit_action  = 0, //FSM_ACTION_DO_NOTHING
	 .transitions  = (fsm_transition_t[]){
		 /** 460 Timeout event will fire this transition */
		 { .event_type = 0, .guard = NULL  , .action = 0, .target_state = &s_4_1}, //FSM_ACTION_DO_NOTHING
		 /** 458 NAS Security mode COMPLETE */
		 /** 460 A real event */
		 { .event_type = 2, .guard = &g_4_2, .action = 2, .target_state = &s_4_2}  //FSM_ACTION_RESET_TIMER
	 },
	 .transitions_count = 2
 };
/** 487
 * Create a new FSM for this rule
 */
static void *create_new_fsm_4(){
		 return fsm_init( &s_4_0, &s_4_1, &s_4_2, &s_4_3, EVENTS_COUNT_4, sizeof( _msg_t_4 ) );//init, error, final, inconclusive, events_count
 }//end function
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
			 .id               = 4,
			 .type_id          = 4,
			 .type_string      = "FORWARD",
			 .events_count     = EVENTS_COUNT_4,
			 .description      = "Forwarding NAS security mode COMPLETE that answers to NAS security mode COMMAND ",
			 .if_satisfied     = forward_packet_if_satisfied,
			 .if_not_satisfied = NULL,
			 .proto_atts_count = PROTO_ATTS_COUNT_4,
			 .proto_atts       = proto_atts_4,
			 .proto_atts_events= proto_atts_events_4,
			 .excluded_filter  = excluded_filter_4,
			 .create_instance  = &create_new_fsm_4,
			 .hash_message     = &_allocate_msg_t_4,
			 .version          = &version,
		 }
	 };
	 *rules_arr = rules;
	 return 1;
 }