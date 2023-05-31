
/** 1207
 * This file is generated automatically on 2023-05-31 11:07:50
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


 //======================================RULE 3======================================
 #define EVENTS_COUNT_3 2

 #define PROTO_ATTS_COUNT_3 2

/** 862
 * Proto_atts for rule 3
 */

 static proto_attribute_t proto_atts_3[ PROTO_ATTS_COUNT_3 ] = {{.proto = "sctp", .proto_id = 304, .att = "ch_type", .att_id = 5, .data_type = 0, .dpi_type = 1},
 {.proto = "sctp", .proto_id = 304, .att = "dest_port", .att_id = 2, .data_type = 0, .dpi_type = 2}};
/** 874
 * Detail of proto_atts for each event
 */

 static mmt_array_t proto_atts_events_3[ 3 ] = { {.elements_count = 0, .data = NULL}, 
	 {//event_1
		 .elements_count = 2,
		 .data = (void* []) { &proto_atts_3[ 0 ] ,  &proto_atts_3[ 1 ] }
	 },
	 {//event_2
		 .elements_count = 0,
		 .data = NULL
	 } 
 };//end proto_atts_events_

 static mmt_array_t excluded_filter_3[ 3 ] = { {.elements_count = 0, .data = NULL}, 
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
typedef struct _msg_struct_3{
	 uint16_t _sctp_ch_type;
	 uint16_t _sctp_dest_port;
 }_msg_t_3;
/** 551
 * Create an instance of _msg_t_3
 */
static _msg_t_3 _m3;
 static void _allocate_msg_t_3( const char* proto, const char* att, uint16_t index ){
	 if( strcmp( proto, "sctp" ) == 0 && strcmp( att, "ch_type" ) == 0 ){ _m3._sctp_ch_type = index; return; }
	 if( strcmp( proto, "sctp" ) == 0 && strcmp( att, "dest_port" ) == 0 ){ _m3._sctp_dest_port = index; return; }
 }
/** 92
 * Rule 3, event 1
  * From UE and NAS-5G packets
 */
static inline int g_3_1( const message_t *msg, const fsm_t *fsm ){
	 if( unlikely( msg == NULL || fsm == NULL )) return 0;
	 const message_t *his_msg;
	 const void *data;/* 57 */

	 data = get_element_data_message_t( msg, _m3._sctp_ch_type );
	 double _sctp_ch_type = 0;
	 if (data != NULL)  _sctp_ch_type = *(double*) data;/* 57 */

	 data = get_element_data_message_t( msg, _m3._sctp_dest_port );
	 double _sctp_dest_port = 0;
	 if (data != NULL)  _sctp_dest_port = *(double*) data;

	 return ((_sctp_dest_port == 38412) && (_sctp_ch_type == 0));
 }
 
/** 92
 * Rule 3, event 2
  * 
 */
static inline int g_3_2( const message_t *msg, const fsm_t *fsm ){

	 return (1);
 }
 
/** 410
 * States of FSM for rule 3
 */

/** 411
 * Predefine list of states: init, fail, pass, ...
 */
static fsm_state_t s_3_0, s_3_1, s_3_2, s_3_3, s_3_4;
/** 424
 * Initialize states: init, error, final, ...
 */
static fsm_state_t
/** 430
 * initial state
 */
 s_3_0 = {
	 .delay        = {.time_min = 0, .time_max = 0, .counter_min = 0, .counter_max = 0},
	 .is_temporary = 0,
	 .description  = "Inject only SCTP packets from UE -> Core but not inversed direction",
	 .entry_action = 0, //FSM_ACTION_DO_NOTHING
	 .exit_action  = 1, //FSM_ACTION_CREATE_INSTANCE
	 .transitions  = (fsm_transition_t[]){
		 /** 458 From UE and NAS-5G packets */
		 /** 460 A real event */
		 { .event_type = 1, .guard = &g_3_1, .action = 1, .target_state = &s_3_4}  //FSM_ACTION_CREATE_INSTANCE
	 },
	 .transitions_count = 1
 },
/** 430
 * timeout/error state
 */
 s_3_1 = {
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
 s_3_2 = {
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
 s_3_3 = {
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
 s_3_4 = {
	 .delay        = {.time_min = 0LL, .time_max = 0LL, .counter_min = 0LL, .counter_max = 0LL},
	 .is_temporary = 1,
	 .description  = "Inject only SCTP packets from UE -> Core but not inversed direction",
	 .entry_action = 0, //FSM_ACTION_DO_NOTHING
	 .exit_action  = 0, //FSM_ACTION_DO_NOTHING
	 .transitions  = (fsm_transition_t[]){
		 /** 460 Timeout event will fire this transition */
		 { .event_type = 0, .guard = NULL  , .action = 0, .target_state = &s_3_1}, //FSM_ACTION_DO_NOTHING
		 /** 460 A real event */
		 { .event_type = 2, .guard = &g_3_2, .action = 2, .target_state = &s_3_2}  //FSM_ACTION_RESET_TIMER
	 },
	 .transitions_count = 2
 };
/** 487
 * Create a new FSM for this rule
 */
static void *create_new_fsm_3(){
		 return fsm_init( &s_3_0, &s_3_1, &s_3_2, &s_3_3, EVENTS_COUNT_3, sizeof( _msg_t_3 ) );//init, error, final, inconclusive, events_count
 }//end function

 //======================================RULE 104======================================
 #define EVENTS_COUNT_104 2

 #define PROTO_ATTS_COUNT_104 1

/** 862
 * Proto_atts for rule 104
 */

 static proto_attribute_t proto_atts_104[ PROTO_ATTS_COUNT_104 ] = {{.proto = "udp", .proto_id = 376, .att = "dest_port", .att_id = 2, .data_type = 0, .dpi_type = 2}};
/** 874
 * Detail of proto_atts for each event
 */

 static mmt_array_t proto_atts_events_104[ 3 ] = { {.elements_count = 0, .data = NULL}, 
	 {//event_1
		 .elements_count = 1,
		 .data = (void* []) { &proto_atts_104[ 0 ] }
	 },
	 {//event_2
		 .elements_count = 0,
		 .data = NULL
	 } 
 };//end proto_atts_events_

 static mmt_array_t excluded_filter_104[ 3 ] = { {.elements_count = 0, .data = NULL}, 
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
typedef struct _msg_struct_104{
	 uint16_t _udp_dest_port;
 }_msg_t_104;
/** 551
 * Create an instance of _msg_t_104
 */
static _msg_t_104 _m104;
 static void _allocate_msg_t_104( const char* proto, const char* att, uint16_t index ){
	 if( strcmp( proto, "udp" ) == 0 && strcmp( att, "dest_port" ) == 0 ){ _m104._udp_dest_port = index; return; }
 }
/** 92
 * Rule 104, event 1
  * From UE and GTP packets
 */
static inline int g_104_1( const message_t *msg, const fsm_t *fsm ){
	 if( unlikely( msg == NULL || fsm == NULL )) return 0;
	 const message_t *his_msg;
	 const void *data;/* 57 */

	 data = get_element_data_message_t( msg, _m104._udp_dest_port );
	 double _udp_dest_port = 0;
	 if (data != NULL)  _udp_dest_port = *(double*) data;

	 return ((_udp_dest_port == 2152));
 }
 
/** 92
 * Rule 104, event 2
  * 
 */
static inline int g_104_2( const message_t *msg, const fsm_t *fsm ){

	 return (1);
 }
 
/** 410
 * States of FSM for rule 104
 */

/** 411
 * Predefine list of states: init, fail, pass, ...
 */
static fsm_state_t s_104_0, s_104_1, s_104_2, s_104_3, s_104_4;
/** 424
 * Initialize states: init, error, final, ...
 */
static fsm_state_t
/** 430
 * initial state
 */
 s_104_0 = {
	 .delay        = {.time_min = 0, .time_max = 0, .counter_min = 0, .counter_max = 0},
	 .is_temporary = 0,
	 .description  = "Inject only UDP packets from UE -> Core but not inversed direction",
	 .entry_action = 0, //FSM_ACTION_DO_NOTHING
	 .exit_action  = 1, //FSM_ACTION_CREATE_INSTANCE
	 .transitions  = (fsm_transition_t[]){
		 /** 458 From UE and GTP packets */
		 /** 460 A real event */
		 { .event_type = 1, .guard = &g_104_1, .action = 1, .target_state = &s_104_4}  //FSM_ACTION_CREATE_INSTANCE
	 },
	 .transitions_count = 1
 },
/** 430
 * timeout/error state
 */
 s_104_1 = {
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
 s_104_2 = {
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
 s_104_3 = {
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
 s_104_4 = {
	 .delay        = {.time_min = 0LL, .time_max = 0LL, .counter_min = 0LL, .counter_max = 0LL},
	 .is_temporary = 1,
	 .description  = "Inject only UDP packets from UE -> Core but not inversed direction",
	 .entry_action = 0, //FSM_ACTION_DO_NOTHING
	 .exit_action  = 0, //FSM_ACTION_DO_NOTHING
	 .transitions  = (fsm_transition_t[]){
		 /** 460 Timeout event will fire this transition */
		 { .event_type = 0, .guard = NULL  , .action = 0, .target_state = &s_104_1}, //FSM_ACTION_DO_NOTHING
		 /** 460 A real event */
		 { .event_type = 2, .guard = &g_104_2, .action = 2, .target_state = &s_104_2}  //FSM_ACTION_RESET_TIMER
	 },
	 .transitions_count = 2
 };
/** 487
 * Create a new FSM for this rule
 */
static void *create_new_fsm_104(){
		 return fsm_init( &s_104_0, &s_104_1, &s_104_2, &s_104_3, EVENTS_COUNT_104, sizeof( _msg_t_104 ) );//init, error, final, inconclusive, events_count
 }//end function
/** 609
 * Moment the rules being encoded
  * PUBLIC API
 */

static const rule_version_info_t version = {.created_date=1685524070, .hash = "dcce935", .number="0.0.6", .index=600, .dpi="1.7.8 (68bd7d93)"};
const rule_version_info_t * mmt_sec_get_rule_version_info(){ return &version;};

 //======================================GENERAL======================================
/** 623
 * Information of 2 rules
  * PUBLIC API
 */
size_t mmt_sec_get_plugin_info( const rule_info_t **rules_arr ){
	  static const rule_info_t rules[] = (rule_info_t[]){
		 {
			 .id               = 3,
			 .type_id          = 4,
			 .type_string      = "FORWARD",
			 .events_count     = EVENTS_COUNT_3,
			 .description      = "Inject only SCTP packets from UE -> Core but not inversed direction",
			 .if_satisfied     = forward_packet_if_satisfied,
			 .if_not_satisfied = NULL,
			 .proto_atts_count = PROTO_ATTS_COUNT_3,
			 .proto_atts       = proto_atts_3,
			 .proto_atts_events= proto_atts_events_3,
			 .excluded_filter  = excluded_filter_3,
			 .create_instance  = &create_new_fsm_3,
			 .hash_message     = &_allocate_msg_t_3,
			 .version          = &version,
		 },
		 {
			 .id               = 104,
			 .type_id          = 4,
			 .type_string      = "FORWARD",
			 .events_count     = EVENTS_COUNT_104,
			 .description      = "Inject only UDP packets from UE -> Core but not inversed direction",
			 .if_satisfied     = forward_packet_if_satisfied,
			 .if_not_satisfied = NULL,
			 .proto_atts_count = PROTO_ATTS_COUNT_104,
			 .proto_atts       = proto_atts_104,
			 .proto_atts_events= proto_atts_events_104,
			 .excluded_filter  = excluded_filter_104,
			 .create_instance  = &create_new_fsm_104,
			 .hash_message     = &_allocate_msg_t_104,
			 .version          = &version,
		 }
	 };
	 *rules_arr = rules;
	 return 2;
 }