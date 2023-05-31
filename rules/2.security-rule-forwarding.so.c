
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


static void em_forward(
      const rule_info_t *rule, int verdict, uint64_t timestamp, 
      uint64_t counter, const mmt_array_t * const trace ){
   const char* trace_str = mmt_convert_execution_trace_to_json_string( trace, rule );
   forward_packet();
   //old ran_ue_id
   uint64_t ran_ue_id = get_numeric_value( PROTO_NGAP, NGAP_ATT_RAN_UE_ID, 1, trace );
   for( int i=0; i<1; i++ ){
      set_numeric_value( PROTO_NGAP, NGAP_ATT_RAN_UE_ID, ran_ue_id + i + 100 );
      forward_packet();
   }
}

void on_load(){
	printf("Loaded successfully rule 1");
}

void on_unload(){
	printf("Unloaded successfully rule 1");
}

bool em_check(){
	sleep(1);
	printf("sleep 1 second");
	return true;
}




 //======================================RULE 2======================================
 #define EVENTS_COUNT_2 2

 #define PROTO_ATTS_COUNT_2 3

/** 862
 * Proto_atts for rule 2
 */

 static proto_attribute_t proto_atts_2[ PROTO_ATTS_COUNT_2 ] = {{.proto = "ip", .proto_id = 178, .att = "src", .att_id = 13, .data_type = 1, .dpi_type = 8},
 {.proto = "sctp", .proto_id = 304, .att = "ch_type", .att_id = 5, .data_type = 0, .dpi_type = 1},
 {.proto = "sctp", .proto_id = 304, .att = "dest_port", .att_id = 2, .data_type = 0, .dpi_type = 2}};
/** 874
 * Detail of proto_atts for each event
 */

 static mmt_array_t proto_atts_events_2[ 3 ] = { {.elements_count = 0, .data = NULL}, 
	 {//event_1
		 .elements_count = 3,
		 .data = (void* []) { &proto_atts_2[ 0 ] ,  &proto_atts_2[ 1 ] ,  &proto_atts_2[ 2 ] }
	 },
	 {//event_2
		 .elements_count = 0,
		 .data = NULL
	 } 
 };//end proto_atts_events_

 static mmt_array_t excluded_filter_2[ 3 ] = { {.elements_count = 0, .data = NULL}, 
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
typedef struct _msg_struct_2{
	 uint16_t _ip_src;
	 uint16_t _sctp_ch_type;
	 uint16_t _sctp_dest_port;
 }_msg_t_2;
/** 551
 * Create an instance of _msg_t_2
 */
static _msg_t_2 _m2;
 static void _allocate_msg_t_2( const char* proto, const char* att, uint16_t index ){
	 if( strcmp( proto, "ip" ) == 0 && strcmp( att, "src" ) == 0 ){ _m2._ip_src = index; return; }
	 if( strcmp( proto, "sctp" ) == 0 && strcmp( att, "ch_type" ) == 0 ){ _m2._sctp_ch_type = index; return; }
	 if( strcmp( proto, "sctp" ) == 0 && strcmp( att, "dest_port" ) == 0 ){ _m2._sctp_dest_port = index; return; }
 }
/** 92
 * Rule 2, event 1
  * From UE and NAS-5G packets
 */
static inline int g_2_1( const message_t *msg, const fsm_t *fsm ){
	 if( unlikely( msg == NULL || fsm == NULL )) return 0;
	 const message_t *his_msg;
	 const void *data;/* 57 */

	 data = get_element_data_message_t( msg, _m2._ip_src );
	 const char *_ip_src = (char *) data;/* 57 */

	 data = get_element_data_message_t( msg, _m2._sctp_ch_type );
	 double _sctp_ch_type = 0;
	 if (data != NULL)  _sctp_ch_type = *(double*) data;/* 57 */

	 data = get_element_data_message_t( msg, _m2._sctp_dest_port );
	 double _sctp_dest_port = 0;
	 if (data != NULL)  _sctp_dest_port = *(double*) data;

	 return ((is_same_ipv4(_ip_src , "127.0.0.1")) && ((_sctp_dest_port == 38412) && (_sctp_ch_type == 0)));
 }
 
/** 92
 * Rule 2, event 2
  * 
 */
static inline int g_2_2( const message_t *msg, const fsm_t *fsm ){

	 return (1);
 }
 
/** 410
 * States of FSM for rule 2
 */

/** 411
 * Predefine list of states: init, fail, pass, ...
 */
static fsm_state_t s_2_0, s_2_1, s_2_2, s_2_3, s_2_4;
/** 424
 * Initialize states: init, error, final, ...
 */
static fsm_state_t
/** 430
 * initial state
 */
 s_2_0 = {
	 .delay        = {.time_min = 0, .time_max = 0, .counter_min = 0, .counter_max = 0},
	 .is_temporary = 0,
	 .description  = "Inject only packet from UE -> Core but not inversed direction",
	 .entry_action = 0, //FSM_ACTION_DO_NOTHING
	 .exit_action  = 1, //FSM_ACTION_CREATE_INSTANCE
	 .transitions  = (fsm_transition_t[]){
		 /** 458 From UE and NAS-5G packets */
		 /** 460 A real event */
		 { .event_type = 1, .guard = &g_2_1, .action = 1, .target_state = &s_2_4}  //FSM_ACTION_CREATE_INSTANCE
	 },
	 .transitions_count = 1
 },
/** 430
 * timeout/error state
 */
 s_2_1 = {
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
 s_2_2 = {
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
 s_2_3 = {
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
 s_2_4 = {
	 .delay        = {.time_min = 0LL, .time_max = 0LL, .counter_min = 0LL, .counter_max = 0LL},
	 .is_temporary = 1,
	 .description  = "Inject only packet from UE -> Core but not inversed direction",
	 .entry_action = 0, //FSM_ACTION_DO_NOTHING
	 .exit_action  = 0, //FSM_ACTION_DO_NOTHING
	 .transitions  = (fsm_transition_t[]){
		 /** 460 Timeout event will fire this transition */
		 { .event_type = 0, .guard = NULL  , .action = 0, .target_state = &s_2_1}, //FSM_ACTION_DO_NOTHING
		 /** 460 A real event */
		 { .event_type = 2, .guard = &g_2_2, .action = 2, .target_state = &s_2_2}  //FSM_ACTION_RESET_TIMER
	 },
	 .transitions_count = 2
 };
/** 487
 * Create a new FSM for this rule
 */
static void *create_new_fsm_2(){
		 return fsm_init( &s_2_0, &s_2_1, &s_2_2, &s_2_3, EVENTS_COUNT_2, sizeof( _msg_t_2 ) );//init, error, final, inconclusive, events_count
 }//end function
/** 609
 * Moment the rules being encoded
  * PUBLIC API
 */

static const rule_version_info_t version = {.created_date=1685524070, .hash = "dcce935", .number="0.0.6", .index=600, .dpi="1.7.8 (68bd7d93)"};
const rule_version_info_t * mmt_sec_get_rule_version_info(){ return &version;};

 //======================================GENERAL======================================
/** 623
 * Information of 1 rules
  * PUBLIC API
 */
size_t mmt_sec_get_plugin_info( const rule_info_t **rules_arr ){
	  static const rule_info_t rules[] = (rule_info_t[]){
		 {
			 .id               = 2,
			 .type_id          = 4,
			 .type_string      = "FORWARD",
			 .events_count     = EVENTS_COUNT_2,
			 .description      = "Inject only packet from UE -> Core but not inversed direction",
			 .if_satisfied     = forward_packet_if_satisfied,
			 .if_not_satisfied = NULL,
			 .proto_atts_count = PROTO_ATTS_COUNT_2,
			 .proto_atts       = proto_atts_2,
			 .proto_atts_events= proto_atts_events_2,
			 .excluded_filter  = excluded_filter_2,
			 .create_instance  = &create_new_fsm_2,
			 .hash_message     = &_allocate_msg_t_2,
			 .version          = &version,
		 }
	 };
	 *rules_arr = rules;
	 return 1;
 }