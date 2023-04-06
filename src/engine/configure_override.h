/*
 * configure_override.h
 *
 *  Created on: Apr 23, 2018
 *          by: Huu Nghia Nguyen
 */

#ifndef SRC_CONFIGURE_OVERRIDE_H_
#define SRC_CONFIGURE_OVERRIDE_H_

#include "configure.h"
#include "../lib/macro_apply.h"

/**
 * Data type using by the attributes of the configuration
 */
typedef enum{
   NO_SUPPORT,
   BOOL,
   UINT16_T,
   UINT32_T,
   CHAR_STAR,
   LIST
} data_type_t;

typedef struct identity_struct{
	int val;
	data_type_t data_type;
	const char *ident;
}identity_t;

const identity_t* conf_get_identity_from_string( const char * ident_str );

/**
 * Override an attribute in configuration.
 * @param
 * @param ident: identifier of element will be overridden.
 * @param value: value will be overridden only if the value is different with the current one of the element.
 * @return 0 if the value has been overridden, otherwise false
 */
int conf_override_element( config_t*, const char* ident, const char *value );

/**
 * Override an attribute in configuration.
 * @param conf
 * @param ident_val
 * @param value_str
 * @return
 */
bool conf_override_element_by_id( config_t *conf, int ident_val, const char *value_str );

/**
 * Check if data_value is suitable for an identity.
 * @param ident
 * @param data_value
 * @return NULL if yes, otherwise, a text representing error reasons.
 */
const char* conf_validate_data_value( const identity_t *ident, const char *data_value );

void conf_print_identities_list();



#define _FIRST(  a, ... )   a
#define FIRST(  a )  _FIRST  a

#define _SECOND( a, b, c, d )  {.val = a, .data_type = d, .ident = b}
#define SECOND( a )  _SECOND a

#define _CASE( a, b, c, d ) case a: return c;
#define CASE( a )    _CASE a

#define COMMA() ,
#define EMPTY()

#define DECLARE_CONF_ATT( ... )                                           \
	                                                                      \
/*list of identities by number*/                                          \
typedef enum {                                                            \
	APPLY( COMMA,  FIRST, __VA_ARGS__ )                                   \
}config_identity_t;                                                       \
                                                                          \
/*list of identities*/                                                    \
static inline size_t conf_get_identities( const identity_t **lst ){       \
	static identity_t identities[ COUNT_ARGS( __VA_ARGS__ ) ] = {         \
		APPLY( COMMA, SECOND, __VA_ARGS__ )                               \
    };                                                                    \
    if( lst != NULL ) *lst = identities;                                  \
    return COUNT_ARGS( __VA_ARGS__ );                                     \
};                                                                        \
                                                                          \
/*get a file of config_t by identities number */                      \
static inline void* conf_get_ident_attribute_field(                       \
	config_t *conf, config_identity_t x ){                            \
	switch( x ){                                                          \
		APPLY( EMPTY, CASE, __VA_ARGS__ )                                 \
	}                                                                     \
	return NULL;                                                          \
}

/**
 * In the following declaration, each line uses the structure:
 *  (ident-number, ident-string, pointer-field, data-type)
 * - ident-number: is used to define enum element
 * - ident-string: is string of configuration attribute.
 *    They are the same as in mmt-5greplay.conf. Its level is separated by dot, for example:
 *    "input.mode" will represent "mode" inside the "input" block
 * - pointer-field: is a pointer pointing to a field of "conf" variable having "config_t" type
 * - data-type: is data type of the attribute. It can be bool, uint16_t, uint32_t or char*
 */
DECLARE_CONF_ATT(
	(CONF_ATT__NONE, "no-support", NULL, NO_SUPPORT),
	//general
	(CONF_ATT__STACK_TYPE,   "stack-type", &conf->stack_type, UINT32_T),
	(CONF_ATT__INPUT__DPDK_OPTION, "dpdk-option", &conf->dpdk_options, CHAR_STAR),

	//input
	(CONF_ATT__INPUT__MODE,        "input.mode",        &conf->input->input_mode,   UINT16_T),
	(CONF_ATT__INPUT__SOURCE,      "input.source",      &conf->input->input_source, CHAR_STAR),
	(CONF_ATT__INPUT__SNAP_LEN,    "input.snap-len",    &conf->input->snap_len,     UINT16_T),

	//output
	(CONF_ATT__OUTPUT__ENABLE,            "output.enable",             &conf->output->is_enable,             BOOL),
	(CONF_ATT__OUTPUT__OUTPUT_DIR,        "output.output-dir",         &conf->output->output_dir,            CHAR_STAR),
	(CONF_ATT__OUTPUT__REPORT_DESCRIPTION,"output.report-description", &conf->output->is_report_description, BOOL),
	(CONF_ATT__OUTPUT__SAMPLE_INTERVAL,   "output.sample-interval",    &conf->output->sample_interval,       UINT16_T),

	//engine
	(CONF_ATT__ENGINE__THREAD_NB,         "engine.thread-nb",          &conf->engine->threads_size,   UINT16_T),
	(CONF_ATT__ENGINE__EXCLUDE_RULES,     "engine.exclude-rules",      &conf->engine->excluded_rules, CHAR_STAR),
	(CONF_ATT__ENGINE__RULES_MASK,        "engine.rules-mask",         &conf->engine->rules_mask,     CHAR_STAR),
	(CONF_ATT__ENGINE__MAX_INSTANCES,     "engine.max-instances",      &conf->engine->max_instances,  UINT32_T ),

	(CONF_ATT__MEMPOOL__MAX_MESSAGE_SIZE, "mempool.max-message-size", &conf->mempool->max_message_size, UINT32_T),
	(CONF_ATT__MEMPOOL__MAX_BYTES,        "mempool.max-bytess",       &conf->mempool->max_bytes,        UINT16_T),
	(CONF_ATT__MEMPOOL__MAX_ELEMENTS,     "mempool.max-elements",     &conf->mempool->max_elements,     UINT16_T),
	(CONF_ATT__MEMPOOL__SMP_RING_SIZE,    "mempool.smp-ring-size",    &conf->mempool->smp_ring_size,    UINT32_T ),

	(CONF_ATT__FORWARD__ENABLE,    "forward.enable",     &conf->forward->is_enable,  BOOL),
	(CONF_ATT__FORWARD__OUTPUT_NIC,"forward.output-nic", &conf->forward->output_nic, CHAR_STAR),
	(CONF_ATT__FORWARD__SNAP_LEN,  "forward.snap-len",   &conf->forward->snap_len,   UINT16_T),
	(CONF_ATT__FORWARD__NB_COPIES, "forward.nb-copies",  &conf->forward->nb_copies,  UINT32_T),
	(CONF_ATT__FORWARD__DEFAULT,   "forward.default",    &conf->forward->default_action,  UINT32_T),
	(CONF_ATT__FORWARD__TARGET_PROTOCOLS,   "forward.target-protocols", &conf->forward->targets,  LIST),
	(CONF_ATT__FORWARD__TARGET_HOSTS,       "forward.target-hosts",     &conf->forward->targets,  LIST),
	(CONF_ATT__FORWARD__TARGET_PORTS,       "forward.target-ports",     &conf->forward->targets,  LIST),

	(CONF_ATT__DUMP_PACKET__ENABLE,      "dump-packet.enable",      &conf->dump_packet->is_enable,   BOOL),
	(CONF_ATT__DUMP_PACKET__OUTPUT_FILE, "dump-packet.output-file", &conf->dump_packet->output_file, CHAR_STAR)
)

/**
 * Get identity_t object from a number ID.
 * @param id
 * @return
 */
static inline const identity_t* conf_get_identity_from_id( int id ){
	const identity_t *identities;
	size_t nb_parameters = conf_get_identities( &identities );

	if( id < 0 || id >= nb_parameters )
		return NULL;

	return &identities[ id ];
}


static inline const char* conf_get_name_from_id( config_identity_t id ){
	return conf_get_identity_from_id( id )->ident;
}

uint32_t conf_get_number_value( config_identity_t id );

bool conf_load_config( const char *path );

const config_t *conf_get();

void conf_release();


#endif /* SRC_CONFIGURE_OVERRIDE_H_ */
