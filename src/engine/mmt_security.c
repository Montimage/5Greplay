/*
 * engine.c
 *
 *  Created on: Mar 17, 2017
 *  Created by: Huu Nghia NGUYEN <huunghia.nguyen@montimage.com>
 */


#include <signal.h>
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include <mmt_core.h>
#include <tcpip/mmt_tcpip.h>

#include "mmt_security.h"

#include "mmt_fsm.h"
#include "plugins_engine.h"
#include "rule_verif_engine.h"
#include "expression.h"
#include "rule.h"
#include "plugin_header.h"

#include "mmt_single_security.h"
#include "mmt_smp_security.h"

//maximal number of rules we support
// this value can be freely changed.
// it is used to reserve a static memory segment
#ifndef MAX_RULES_COUNT
#define MAX_RULES_COUNT      100000
#endif

//maximal number of protocol attributes we support
#ifndef MAX_PROTO_ATTS_COUNT
#define MAX_PROTO_ATTS_COUNT  64 //currently limited by BIT operation on an uint64_t
#endif

#ifdef MODULE_ADD_OR_RM_RULES_RUNTIME
#include <pthread.h>

/**
 * This spinlock is used when we need to add/remove rules at runtime
 */
static pthread_spinlock_t spin_lock;
#endif

static const rule_info_t *rules[MAX_RULES_COUNT]; //total rules set
static size_t rules_count = 0;			         //total number of rules

static const proto_attribute_t *proto_atts[MAX_PROTO_ATTS_COUNT]; //total proto_atts set
static size_t proto_atts_count = 0;                 //total number of proto_atts

//marking whether the function mmt_sec_init is called
static bool is_init = NO;

//string size of an alert in JSON format
#define MAX_MSG_SIZE 10000


struct mmt_sec_handler_struct{
	void *sec_handler;

	void (*process)( void *, message_t *);

	int threads_count;

#ifdef MODULE_ADD_OR_RM_RULES_RUNTIME
	volatile int rule_processing_status;
#endif
};


#ifdef MODULE_ADD_OR_RM_RULES_RUNTIME
//TODO: limit to 100 mmt-engine handlers
#define MAX_HANDLERS_COUNT 100
static mmt_sec_handler_t *mmt_sec_handlers[ MAX_HANDLERS_COUNT ];
static int mmt_sec_handlers_count = 0;
#endif


/**
 * Naif minimal perfect hash function
 *
 * it's not necessary to use hash if #proto_atts_count < 50
 * Tested on 10 Oct 2017.
 */
uint16_t _mmt_sec_hash_proto_attribute_without_lock( uint32_t proto_id, uint32_t att_id ){
	int i;
	for( i=0; i<proto_atts_count; i++ )
		if( proto_atts[ i ]->att_id  == att_id && proto_atts[ i ]->proto_id  == proto_id ){
			return i;
		}

	ABORT( "Attribute %d.%d has not been registered in MMT-5Greplay", proto_id, att_id );
	return 0;
}

void _filter_rules( const char *rule_mask ){
	uint32_t *rule_range, rule_id;
	int i, j, k, count;

	if( rule_mask == NULL || strlen( rule_mask ) == 0 )
		return;

	//rules are not verified
	count = expand_number_range( rule_mask, &rule_range );
	if( count > 0 ){
		//move ignored rules to the end
		//rule_ptr will ignored the last n rules
		for( j=count-1; j>=0; j-- ){
			rule_id = rule_range[ j ];
			if( rules_count == 0 )
				return;

			for( k=rules_count-1; k>=0; k-- )
				if( rule_id == rules[k]->id ){
					//ignore this rule: rules_array[rules_count--]
					rules_count --;

					rules[k] = rules[ rules_count ];;
					break;
				}
		}
	}
	mmt_mem_free( rule_range );
}

static inline void _iterate_proto_atts( void *key, void *data, void *user_data, size_t index, size_t total ){
	proto_atts[ index ] = data;
	//free the key being created on line 185 in function _get_unique_proto_attts
	mmt_mem_free( key );
}

static inline void _get_unique_proto_attts( ){
	const rule_info_t *rule;
	size_t i, j;

	mmt_map_t *map = mmt_map_init( compare_uint64_t );
	uint64_t *proto_att_key;
	const proto_attribute_t *me, *old;

	//for each rule
	for( i=0; i<rules_count; i++ ){
		rule = rules[i];
		for( j=0; j<rule->proto_atts_count; j++ ){
			me = &rule->proto_atts[j];
			proto_att_key = mmt_mem_alloc( sizeof( uint64_t) );
			*proto_att_key = simple_hash_64( me->proto_id, me->att_id );

			if( (old = mmt_map_set_data( map, proto_att_key, (void *)me, NO )) != NULL ){
				//already exist
				mmt_mem_free( proto_att_key );
			}
		}
	}

	proto_atts_count = mmt_map_count( map );
	//check limit
	if( proto_atts_count > MAX_PROTO_ATTS_COUNT )
		ABORT( "A single mmt_security cannot handler more than %d different proto.att. You might need to use mmt_smp_sec to divide work load.", MAX_PROTO_ATTS_COUNT );

	mmt_map_iterate( map, _iterate_proto_atts, NULL );

	mmt_map_free( map, NO );
}

/**
 * Attribute an unique number for each pair proto/att inside the current rules set
 */
static inline void _update_rules_hash( ){
	int index, i, j;
	const rule_info_t *rule;
	const proto_attribute_t *p;
	uint16_t hash;
	//for each index
	for( index=0; index< proto_atts_count; index++ ){
		p = proto_atts[ index ];
		hash = _mmt_sec_hash_proto_attribute_without_lock( p->proto_id, p->att_id);
		//for each rule
		for( i=0; i< rules_count; i++ ){
			rule = rules[ i ];
			rule->hash_message( p->proto, p->att, hash );
		}
		DEBUG("%2d <- hash(%3d, %4d) (%s, %s)", hash, p->proto_id, p->att_id, p->proto, p->att );
	}
}


/**
 * PUBLIC API
 * This function inits engine rules
 * @return
 */
int mmt_sec_init( const char* excluded_rules_id ){
	ASSERT( is_init == NO, "mmt_sec_init must be called only once.");

#ifdef MODULE_ADD_OR_RM_RULES_RUNTIME
	pthread_spin_init( &spin_lock, PTHREAD_PROCESS_PRIVATE );
#endif
	size_t i;

	//TODO:
	//mmt_sec_load_default_config();

	//TODO: to remove
	//ABORT("under construction...");

	rule_info_t const* const* rules_arr;
	is_init = YES;

	//get all available rules
	rules_count = load_mmt_sec_rules( &rules_arr );

	//clone rules_arr to rules
	for( i=0; i<rules_count; i++ )
		rules[i] = rules_arr[i];

	//Rules to be disabled
	_filter_rules( excluded_rules_id );

	if( rules_count == 0 ){
		log_write( LOG_WARNING,"There are no rules to verify.");
		return 1;
	}

	_get_unique_proto_attts();
	_update_rules_hash();

	return 0;
}


void mmt_sec_close(){
	rules_count = 0;
	is_init = NO;
	proto_atts_count = 0;
}

/**
 *
 * @param threads_count: if 0, engine will use the lcore of caller
 * @param cores_id
 * @param rules_mask
 * @param verbose
 * @param callback
 * @param user_data
 * @return
 */
mmt_sec_handler_t* mmt_sec_register( size_t threads_count, const uint32_t *cores_id, const char *rules_mask,
		bool verbose, mmt_sec_callback callback, void *args ){

	mmt_sec_handler_t *ret = mmt_mem_alloc(sizeof( mmt_sec_handler_t ));

	size_t i, j;

	//number of threads
	ret->threads_count = threads_count;

	//
	if( unlikely( is_init == NO) )
		ABORT("mmt_sec_init must be called before any mmt_sec_register" );

	if( verbose ){
		if( threads_count == 0 )
			log_write( LOG_INFO, "MMT-5Greplay %s is verifying %zu rules having %zu proto.atts using the main thread",
				mmt_sec_get_version_info(),
				rules_count, proto_atts_count );
		else
			log_write( LOG_INFO, "MMT-5Greplay %s is verifying %zu rules having %zu proto.atts using %zu threads",
							mmt_sec_get_version_info(),
							rules_count, proto_atts_count, threads_count );
	}
	//init mmt-sec to verify the rules
	if( threads_count == 0 ){
		ret->sec_handler = mmt_single_sec_register( rules, rules_count, verbose, callback, args );
		ret->process      = (void *)&mmt_single_sec_process;
	} else {
		ret->sec_handler = mmt_smp_sec_register( threads_count,
												cores_id, rules_mask, verbose, callback, args );
		ret->process = (void *)&mmt_smp_sec_process;
	}

	//save global list of mmt-sec-handlers
#ifdef MODULE_ADD_OR_RM_RULES_RUNTIME
	BEGIN_LOCK_IF_ADD_OR_RM_RULES_RUNTIME( &spin_lock )
	ASSERT( mmt_sec_handlers_count < MAX_HANDLERS_COUNT, "Support maximally %d mmt_security_handler_t", MAX_HANDLERS_COUNT );
	mmt_sec_handlers[ mmt_sec_handlers_count ] = ret;
	mmt_sec_handlers_count ++;
	UNLOCK_IF_ADD_OR_RM_RULES_RUNTIME( &spin_lock )
	END_LOCK_IF_ADD_OR_RM_RULES_RUNTIME
#endif

	return ret;
}

void mmt_sec_set_ignore_remain_flow( mmt_sec_handler_t *handler, bool ignore, uint64_t buffer_size ){
	if( handler->threads_count > 0 )
		mmt_smp_sec_set_ignore_remain_flow( handler->sec_handler, ignore, buffer_size );
	else
		mmt_single_sec_set_ignore_remain_flow( handler->sec_handler, ignore, buffer_size );
}

bool mmt_sec_is_ignore_remain_flow( mmt_sec_handler_t *handler, uint64_t flow_id ){
	if( handler->threads_count > 0 )
		return mmt_smp_is_ignore_remain_flow( handler->sec_handler, flow_id );
	else
		return mmt_single_is_ignore_remain_flow( handler->sec_handler, flow_id );
}

void mmt_sec_process( mmt_sec_handler_t *handler, message_t *msg ){
	handler->process( handler->sec_handler, msg );
}

/**
 * Stop and free mmt_security
 * @param wrapper
 * @return
 */
size_t mmt_sec_unregister( mmt_sec_handler_t* ret ){
	size_t alerts_count = 0;

	if( unlikely( ret == NULL) )
		return 0;

	if( ret->threads_count > 0 )
		alerts_count = mmt_smp_sec_unregister( ret->sec_handler, NO );
	else
		alerts_count = mmt_single_sec_unregister( ret->sec_handler );

	mmt_mem_free( ret );

	return alerts_count;
}

/**
 * Local usage
 * @param rules_array
 * @return
 */
size_t _mmt_sec_get_rules_info_without_lock( rule_info_t const*const**rules_array ){
	size_t ret = 0;
	*rules_array = rules;
	ret          = rules_count;
	return ret;
}

size_t mmt_sec_get_rules_info( rule_info_t const*const**rules_array ){
	size_t ret = 0;
	BEGIN_LOCK_IF_ADD_OR_RM_RULES_RUNTIME( &spin_lock )
	ret = _mmt_sec_get_rules_info_without_lock( rules_array );
	UNLOCK_IF_ADD_OR_RM_RULES_RUNTIME( &spin_lock )
	END_LOCK_IF_ADD_OR_RM_RULES_RUNTIME

	return ret;
}

static inline size_t _copy_plein_text( char *dst, int len, const char* src ){
	size_t size = 0;
	//copy maximal 255 characters
	if( len > 255 )
		len = 255;

	while( len != 0 && *src != '\0' ){
		switch( *src ){
		case '\b': //  Backspace (ascii code 08)
		case '\f': //  Form feed (ascii code 0C)
		case '\n': //  New line
		case '\r': //  Carriage return
		case '\t': //  Tab
		case '\"': //  Double quote
		case '\\': //  Backslash character
		//case '\u': //  unicode
			*dst = '.';
			break;
		default:
			//non printable
			if( !isprint( *src ) )
				*dst = '.';
			else
				*dst = *src;
		}

		src ++;
		dst ++;
		len --;
		size ++;
	}

	return size;
}

#define MAX_STR_SIZE 10000

static const char* _convert_execution_trace_to_json_string( const mmt_array_t *trace, const rule_info_t *rule ){
	static __thread_scope char buffer[ MAX_STR_SIZE + 1 ];
	char *str_ptr, *c_ptr;
	size_t size, i, j, index;
	int total_len;
	const message_t *msg;
	const message_element_t *me;
	bool is_first;
	struct timeval time;
	const mmt_array_t *proto_atts_event; //proto_att of an event
	const proto_attribute_t *pro_ptr;
	double double_val;
	uint8_t *u8_ptr;

	__check_null( trace, NULL );

	buffer[0] = '{';
	buffer[1] = '\0';

	//number of elements in traces <= number of real events + timeout event
#ifdef DEBUG_MODE
	ASSERT( trace->elements_count <= rule->events_count + 1,
			"Impossible: elements_count > events_count (%zu > %d + 1)", trace->elements_count, rule->events_count);
#endif

	total_len = MAX_STR_SIZE;
	str_ptr   = buffer+1;

	for( index=0; index<trace->elements_count; index ++ ){
		msg = trace->data[ index ];
		if( msg == NULL ) continue;

		mmt_sec_decode_timeval( msg->timestamp, &time );

		//seperator of each event
		if( total_len != MAX_STR_SIZE )
			*(str_ptr ++) = ',';

		//event 's detail
		size = snprintf( str_ptr, total_len, "\"event_%zu\":{\"timestamp\":%ld.%06ld,\"counter\":%"PRIu64",\"attributes\":[",
						index,
						time.tv_sec, //timestamp: second
						time.tv_usec, //timestamp: microsecond
						msg->counter );

		is_first = YES;

		//get array of proto_att used in the event having #index
		proto_atts_event = &rule->proto_atts_events[ index ];

		//go into detail of a message
		for( i=0; i<msg->elements_count; i++ ){
			me = &msg->elements[i];

			if( me->data == NULL ) continue;

			//check if #me is used in this event of the rule #rule
			for( j=0; j<proto_atts_event->elements_count; j++ ){
				pro_ptr = (proto_attribute_t *)proto_atts_event->data[ j ];
				if( pro_ptr->att_id == me->att_id && pro_ptr->proto_id == me->proto_id )
					break;
			}

			//not found any variable/proto_att in this event using "me"
			if( j>= proto_atts_event->elements_count )
				continue;

			total_len -= size;
			if( unlikely( total_len <= 0 )){
				break;
			}

			str_ptr += size;

			//do not forget ]
			size = snprintf( str_ptr, total_len, "%s[\"%s.%s\",", //[key, value]
					(is_first? "":","),
					pro_ptr->proto,
					pro_ptr->att);

			str_ptr   += size;
			total_len -= size;

			//pro_ptr->data_type;
			switch( me->data_type ){
			case MMT_SEC_MSG_DATA_TYPE_NUMERIC:
				double_val = *(double *)me->data;

				//do not forget }
				size = snprintf( str_ptr, total_len, "%.5f", double_val );

				c_ptr = str_ptr + size;
				//remove zero at the end, e.g., 10.00 ==> 10
				while( *c_ptr == '0' || *c_ptr == '\0' ){
					c_ptr --;
					//avoid cutting the last number != 0
					if( *c_ptr == '0')
						size --;

					if( *c_ptr == '.'){
						size --;
						break;
					}
				}

				break;

			default:


				u8_ptr = NULL;

				switch( pro_ptr->dpi_type ){
				case MMT_DATA_IP_NET: /**< ip network address constant value */
				case MMT_DATA_IP_ADDR: /**< ip address constant value */
						u8_ptr = (uint8_t *) me->data;
						size   = snprintf(str_ptr, total_len, "\"%d.%d.%d.%d\"",
											u8_ptr[0], u8_ptr[1], u8_ptr[2], u8_ptr[3] );
					break;

					//IPV6 address
				case MMT_DATA_IP6_ADDR:
					u8_ptr = (uint8_t *) me->data;
					char ip_string[ INET6_ADDRSTRLEN ];
					if( inet_ntop(AF_INET6, (void*) u8_ptr, ip_string, INET6_ADDRSTRLEN )){
						size =  sprintf( str_ptr, "\"%s\"", ip_string );
					}
					break;
					//MAC address
				case MMT_DATA_MAC_ADDR:
						u8_ptr = (uint8_t *) me->data;
						size   = snprintf(str_ptr, total_len, "\"%02x:%02x:%02x:%02x:%02x:%02x\"",
								u8_ptr[0], u8_ptr[1], u8_ptr[2], u8_ptr[3], u8_ptr[4], u8_ptr[5] );
					break;
				}// end of switch( pro_ptr->proto_id ){

				//if the attribute is not neither IP nor MAC
				if( u8_ptr == NULL ){
					//TODO: limit output length of one proto.att to 255 bytes
					*str_ptr = '"';
					str_ptr ++;
					size = _copy_plein_text(  str_ptr, total_len - 20, (char *) me->data );
					str_ptr[ size ] = '"';
					size ++;
				}
				//

			}//end of switch( me->data_type )

			//close ] here
			str_ptr += size;
			*str_ptr = ']';
			*(str_ptr + 1 ) = '\0';

			size = 1;

			is_first = NO;
		}

		total_len -= size;
		if( unlikely( total_len <= 0 )){
			log_write( LOG_WARNING,"Buffer size is not enough to contain all attributes");
			//close
			str_ptr += snprintf( str_ptr, total_len, "]}") ;
			break;
		}

		str_ptr += size;
		//end attributes, end event_
		*(str_ptr ++) = ']';
		*(str_ptr ++) = '}';
	}

	*(str_ptr++) = '}'; //end of all
	*(str_ptr++) = '\0';

	return buffer;
}


const char* mmt_convert_execution_trace_to_json_string( const mmt_array_t *trace, const rule_info_t *rule ){
	return _convert_execution_trace_to_json_string( trace, rule );
}

/**
 * PUBLIC API
 * Print information of available rules
 */
void mmt_sec_print_rules_info(){
	size_t i, j, k, size;
	const mmt_array_t *proto_atts;
	const proto_attribute_t *proto;
	struct tm tm;
	char string[ 100000 ], *ch_ptr, tmp_string[ 1000 ];
	string[ 0 ] = '\0';
	ch_ptr = &string[ 0 ];
	int len_remain = sizeof( string );

	BEGIN_LOCK_IF_ADD_OR_RM_RULES_RUNTIME( &spin_lock );
	printf("Found %zu rule%s", rules_count, rules_count<=1? ".": "s." );

	for( i=0; i<rules_count; i++ ){
		printf("\n%zu - Rule id: %d", (i+1), rules[i]->id );
		printf("\n\t- type            : %s",  rules[i]->type_string );
		printf("\n\t- description     : %s",  rules[i]->description );
		printf("\n\t- if_satisfied    : %p",  rules[i]->if_satisfied );
		//for each event
		for(j=0; j<rules[i]->events_count; j++ ){
			printf("\n\t- event %-2zu        ", j+1 );
			//visite each proto/att of one event
			proto_atts = &(rules[i]->proto_atts_events[ j+1 ]);
			for( k=0; k<proto_atts->elements_count; k++ ){
				proto = proto_atts->data[k];
				printf("%c %s.%s (%"PRIu32".%"PRIu32")", k==0?':':',', proto->proto, proto->att,
						proto->proto_id, proto->att_id );

				//add to unique set
				snprintf( tmp_string, sizeof( tmp_string), "%s.%s", proto->proto, proto->att );
				if( strstr( string, tmp_string ) == NULL && len_remain > 0 ){
					size = snprintf( ch_ptr, len_remain, "\"%s.%s\",", proto->proto, proto->att );
					ch_ptr     += size;
					len_remain -= size;
				}
			}
		}

		tm = *localtime(& rules[i]->version->created_date );
		printf("\n\t- version         : %s (%s - %d-%d-%d %d:%d:%d), dpi version %s",
				 rules[i]->version->number,
				 rules[i]->version->hash,
				 tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
				 rules[i]->version->dpi );
	}
	UNLOCK_IF_ADD_OR_RM_RULES_RUNTIME( &spin_lock );
	END_LOCK_IF_ADD_OR_RM_RULES_RUNTIME;

	//remove the last comma
	size = strlen( string );
	if( size > 0 ) string[ size - 1 ] = '\0';

	printf("\n\nProtocols and their attributes used in these rules:\n\t %s\n\n", string );
}


size_t mmt_sec_get_unique_protocol_attributes( proto_attribute_t const *const **proto_atts_array ){
	size_t ret = 0;
	BEGIN_LOCK_IF_ADD_OR_RM_RULES_RUNTIME( &spin_lock )
	*proto_atts_array = proto_atts;
	ret               = proto_atts_count;
	UNLOCK_IF_ADD_OR_RM_RULES_RUNTIME( &spin_lock )
	END_LOCK_IF_ADD_OR_RM_RULES_RUNTIME

	return ret;

}

/**
 * PUBLIC API
 * @param proto_id
 * @param att_id
 * @return
 */
uint16_t mmt_sec_hash_proto_attribute( uint32_t proto_id, uint32_t att_id ){
	uint16_t ret = 0;
	BEGIN_LOCK_IF_ADD_OR_RM_RULES_RUNTIME( &spin_lock )
	ret = _mmt_sec_hash_proto_attribute_without_lock(proto_id, att_id );
	UNLOCK_IF_ADD_OR_RM_RULES_RUNTIME( &spin_lock )
	END_LOCK_IF_ADD_OR_RM_RULES_RUNTIME

	return ret;
}

static inline void _print_proto_atts_hash(){
	size_t i;
	printf("=> got %zu proto_atts\n", proto_atts_count );
	for( i=0; i<proto_atts_count; i++ )
		printf("%3d <= hash(%s, %s)\n", mmt_sec_hash_proto_attribute( proto_atts[i]->proto_id, proto_atts[i]->att_id),
				proto_atts[i]->proto, proto_atts[i]->att );
	fflush( stdout );
}

#ifdef MODULE_ADD_OR_RM_RULES_RUNTIME
//PUBLIC API
//Note: When removing a rule, its proto_att will not be removed from proto_atts list.
// 	=> This will help to keep the order/index of the rested proto_att.
__thread_safe
size_t mmt_sec_remove_rules( size_t rules_to_rm_count, const uint32_t* rules_id_to_rm_set ){
	size_t i, j;
	uint32_t rule_id;
	size_t number_of_rules_will_be_removed = 0;

	DEBUG("Need to remove %zu rule(s)", rules_to_rm_count );

	//remove some rules from #rules
	BEGIN_LOCK_IF_ADD_OR_RM_RULES_RUNTIME( &spin_lock )
		//for each rule tobe removed
		for( i=0; i<rules_to_rm_count; i++ ){
			if( rules_count == 0 )
				break;
			rule_id = rules_id_to_rm_set[ i ];

			//find a rule in #rules having the same id
			for( j=0; j<rules_count; j++ )
				if( rule_id == rules[ j ]->id ){
					//ignore the last rule: rules_array[rules_count--]
					rules_count --;
					//move the last rule to the j-th position
					rules[j] = rules[ rules_count ];
					number_of_rules_will_be_removed ++;
					break;
				}
		}

	if( number_of_rules_will_be_removed > 0 )
		//start remove rules on existing engine handlers
		for( i=0; i<mmt_sec_handlers_count; i++ ){
			if( mmt_sec_handlers[ i ]->threads_count == 0 )
				mmt_single_sec_remove_rules( mmt_sec_handlers[ i ]->sec_handler );
			else
				mmt_smp_sec_remove_rules( mmt_sec_handlers[ i ]->sec_handler );
		}

	UNLOCK_IF_ADD_OR_RM_RULES_RUNTIME( &spin_lock )
	END_LOCK_IF_ADD_OR_RM_RULES_RUNTIME

	EXEC_ONLY_IN_DEBUG_MODE( _print_proto_atts_hash() );

	return number_of_rules_will_be_removed;
}

//PUBLIC API
__thread_safe
size_t mmt_sec_add_rules( const char *rules_mask ){
	//check parameters
	__check_null( rules_mask, 0 );

	uint32_t *rules_mask_range;
	rule_info_t const*const*new_rules_arr;
	const rule_info_t *rule, **tmp_rules;
	const proto_attribute_t **tmp_proto_atts;
	uint16_t hash_number;

	//rules are not verified
	size_t rules_mask_count = get_rules_id_list_in_mask( rules_mask, &rules_mask_range );

	DEBUG("Need to add %s (%zu rules)", rules_mask, rules_mask_count );

	//if no need to update the current rules
	__check_zero( rules_mask_count, 0 );

	//load the current rules set that are existing in its rules folder (either /opt/mmt/engine/rules or ./rules)
	size_t new_rules_count = load_mmt_sec_rules( & new_rules_arr );
	if( unlikely( new_rules_count == 0 )){
		mmt_mem_free( rules_mask_range );
		return 0;
	}

	//maximally: all rules will be added
	const rule_info_t **rules_to_be_added = mmt_mem_alloc( sizeof( void *) * ( new_rules_count ) );
	size_t add_rules_count = 0;
	size_t i, j, k;

	//get set of rules to be added: a rule will be added if
	//1. - it exists in #new_rules_arr (=> it must present in /opt/mmt/engine/rules or ./rules)
	//2. - it exists in #rules_mask
	//3. - it does not exist in #rules (the running rules set)
	for( i=0; i<new_rules_count; i++ ){
		rule = new_rules_arr[ i ];
		//2. if exists in #rules_mask ?
		j = index_of( rule->id, rules_mask_range, rules_mask_count );
		if( j == rules_mask_count ) //not found
			continue;
		//3. if does not exist in #rules?
		for( j=0; j<rules_count; j++ )
			if( rule->id == rules[j]->id )
				break;
		//=> exists
		if( j < rules_count )
			continue;

		//#rule
		rules_to_be_added[ add_rules_count ] = rule;
		add_rules_count ++;
	}

	//if no rule to be added?
	if( unlikely( add_rules_count == 0 )){
		mmt_mem_free( rules_to_be_added );
		mmt_mem_free( rules_mask_range );
		return 0;
	}

	ASSERT( rules_count + add_rules_count <= MAX_RULES_COUNT, "Support maximally %d rules", MAX_RULES_COUNT );

	BEGIN_LOCK_IF_ADD_OR_RM_RULES_RUNTIME( &spin_lock )

	//for each new rule
	for( i=0; i<add_rules_count; i++ ){
		rule = rules_to_be_added[ i ];

		//update new rule
		rules[ rules_count ] = rule;
		rules_count ++;

		//update its set of unique proto/att pair
		//for each proto_att in this rule
		for( j=0; j<rule->proto_atts_count; j++ ){

			//check if proto_att is existing in proto_atts
			for( k=0; k<proto_atts_count; k++ )
				//==> existing
				if( proto_atts[k]->proto_id == rule->proto_atts[j].proto_id
						&& proto_atts[k]->att_id == rule->proto_atts[j].att_id)
					break;

			//does not exist
			if( k == proto_atts_count ){
				proto_atts[ proto_atts_count ] = &rule->proto_atts[ j ];
				proto_atts_count ++;
			}

			ASSERT( proto_atts_count <= MAX_PROTO_ATTS_COUNT,
						"Support maximally %d protocol attributes", MAX_PROTO_ATTS_COUNT );

			//update hash number inside each rule
			hash_number = _mmt_sec_hash_proto_attribute_without_lock(rule->proto_atts[j].proto_id, rule->proto_atts[j].att_id);
			rule->hash_message( rule->proto_atts[j].proto, rule->proto_atts[j].att, hash_number );
		}
	}

	//apply the new rules to all existing handlers
	for( i=0; i<mmt_sec_handlers_count; i++ ){
		if( mmt_sec_handlers[ i ]->threads_count == 0 )
			mmt_single_sec_add_rules( mmt_sec_handlers[ i ]->sec_handler, rules_mask_count, rules_mask_range );
		else
			mmt_smp_sec_add_rules( mmt_sec_handlers[ i ]->sec_handler, rules_mask );
	}

	UNLOCK_IF_ADD_OR_RM_RULES_RUNTIME( &spin_lock )
	END_LOCK_IF_ADD_OR_RM_RULES_RUNTIME

	mmt_mem_free( rules_to_be_added );
	mmt_mem_free( rules_mask_range );

	EXEC_ONLY_IN_DEBUG_MODE( _print_proto_atts_hash() );

	return add_rules_count;
}

#pragma message("Enable module: Add/Remove rules at runtime")
#else
#pragma message("Disable module: Add/Remove rules at runtime")

__thread_safe
size_t mmt_sec_remove_rules( size_t rules_count, const uint32_t* rules_id_set ){
	log_write( LOG_ERR,"Remove rules feature is disable.");
	return 0;
}

__thread_safe
size_t mmt_sec_add_rules( const char *rules_mask ){
	log_write( LOG_ERR,"Add rules feature is disable.");
	return 0;
}

#endif
