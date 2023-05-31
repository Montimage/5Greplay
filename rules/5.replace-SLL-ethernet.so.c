
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


static void em_replace_sll_by_ethernet(
      const rule_info_t *rule, int verdict, uint64_t timestamp, 
      uint64_t counter, const mmt_array_t * const trace ){
   
   const uint8_t ethernet_data[] = {
   	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, //destination
   	0xee, 0xee, 0xee, 0xee, 0xee, 0xee, //source
   	0x08, 0x00 //type IPv4
   }; 
   
   replace_data_at_protocol_id( PROTO_SLL, sizeof(ethernet_data), ethernet_data );
   forward_packet();
}
/** 1240 Create a dummy on_load function as it has not been defined by users in embedded_functions tag*/
void on_load(){}
/** 1245 Create a dummy on_unload function as it has not been defined by users in embedded_functions tag*/
void on_unload(){}


 //======================================RULE 5======================================
 #define EVENTS_COUNT_5 2

 #define PROTO_ATTS_COUNT_5 1

/** 862
 * Proto_atts for rule 5
 */

 static proto_attribute_t proto_atts_5[ PROTO_ATTS_COUNT_5 ] = {{.proto = "sll", .proto_id = 624, .att = "protocol", .att_id = 6, .data_type = 0, .dpi_type = 2}};
/** 874
 * Detail of proto_atts for each event
 */

 static mmt_array_t proto_atts_events_5[ 3 ] = { {.elements_count = 0, .data = NULL}, 
	 {//event_1
		 .elements_count = 1,
		 .data = (void* []) { &proto_atts_5[ 0 ] }
	 },
	 {//event_2
		 .elements_count = 0,
		 .data = NULL
	 } 
 };//end proto_atts_events_

 static mmt_array_t excluded_filter_5[ 3 ] = { {.elements_count = 0, .data = NULL}, 
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
typedef struct _msg_struct_5{
	 uint16_t _sll_protocol;
 }_msg_t_5;
/** 551
 * Create an instance of _msg_t_5
 */
static _msg_t_5 _m5;
 static void _allocate_msg_t_5( const char* proto, const char* att, uint16_t index ){
	 if( strcmp( proto, "sll" ) == 0 && strcmp( att, "protocol" ) == 0 ){ _m5._sll_protocol = index; return; }
 }
/** 92
 * Rule 5, event 1
  * Got SLL packet
 */
static inline int g_5_1( const message_t *msg, const fsm_t *fsm ){
	 if( unlikely( msg == NULL || fsm == NULL )) return 0;
	 const message_t *his_msg;
	 const void *data;/* 57 */

	 data = get_element_data_message_t( msg, _m5._sll_protocol );
	 double _sll_protocol = 0;
	 if (data != NULL)  _sll_protocol = *(double*) data;

	 return (_sll_protocol > 0);
 }
 
/** 92
 * Rule 5, event 2
  * 
 */
static inline int g_5_2( const message_t *msg, const fsm_t *fsm ){

	 return (1);
 }
 
/** 410
 * States of FSM for rule 5
 */

/** 411
 * Predefine list of states: init, fail, pass, ...
 */
static fsm_state_t s_5_0, s_5_1, s_5_2, s_5_3, s_5_4;
/** 424
 * Initialize states: init, error, final, ...
 */
static fsm_state_t
/** 430
 * initial state
 */
 s_5_0 = {
	 .delay        = {.time_min = 0, .time_max = 0, .counter_min = 0, .counter_max = 0},
	 .is_temporary = 0,
	 .description  = "Replace Linux cooked-mode capture (SLL) by Ethernet",
	 .entry_action = 0, //FSM_ACTION_DO_NOTHING
	 .exit_action  = 1, //FSM_ACTION_CREATE_INSTANCE
	 .transitions  = (fsm_transition_t[]){
		 /** 458 Got SLL packet */
		 /** 460 A real event */
		 { .event_type = 1, .guard = &g_5_1, .action = 1, .target_state = &s_5_4}  //FSM_ACTION_CREATE_INSTANCE
	 },
	 .transitions_count = 1
 },
/** 430
 * timeout/error state
 */
 s_5_1 = {
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
 s_5_2 = {
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
 s_5_3 = {
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
 s_5_4 = {
	 .delay        = {.time_min = 0LL, .time_max = 0LL, .counter_min = 0LL, .counter_max = 0LL},
	 .is_temporary = 1,
	 .description  = "Replace Linux cooked-mode capture (SLL) by Ethernet",
	 .entry_action = 0, //FSM_ACTION_DO_NOTHING
	 .exit_action  = 0, //FSM_ACTION_DO_NOTHING
	 .transitions  = (fsm_transition_t[]){
		 /** 460 Timeout event will fire this transition */
		 { .event_type = 0, .guard = NULL  , .action = 0, .target_state = &s_5_1}, //FSM_ACTION_DO_NOTHING
		 /** 460 A real event */
		 { .event_type = 2, .guard = &g_5_2, .action = 2, .target_state = &s_5_2}  //FSM_ACTION_RESET_TIMER
	 },
	 .transitions_count = 2
 };
/** 487
 * Create a new FSM for this rule
 */
static void *create_new_fsm_5(){
		 return fsm_init( &s_5_0, &s_5_1, &s_5_2, &s_5_3, EVENTS_COUNT_5, sizeof( _msg_t_5 ) );//init, error, final, inconclusive, events_count
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
			 .id               = 5,
			 .type_id          = 4,
			 .type_string      = "FORWARD",
			 .events_count     = EVENTS_COUNT_5,
			 .description      = "Replace Linux cooked-mode capture (SLL) by Ethernet",
			 .if_satisfied     = em_replace_sll_by_ethernet,
			 .if_not_satisfied = NULL,
			 .proto_atts_count = PROTO_ATTS_COUNT_5,
			 .proto_atts       = proto_atts_5,
			 .proto_atts_events= proto_atts_events_5,
			 .excluded_filter  = excluded_filter_5,
			 .create_instance  = &create_new_fsm_5,
			 .hash_message     = &_allocate_msg_t_5,
			 .version          = &version,
		 }
	 };
	 *rules_arr = rules;
	 return 1;
 }