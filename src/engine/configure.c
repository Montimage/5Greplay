/*
 * configure.c
 *
 *  Created on: Dec 12, 2017
 *          by: Huu Nghia
 */

#include <confuse.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/stat.h>

#include "../lib/mmt_lib.h"
#include "configure.h"

#define DLT_EN10MB 1/**< Ethernet (10Mb) */

//parse forward default-aciton
bool conf_parse_forward_default_action(int *result, const char *value) {
	if (IS_EQUAL_STRINGS(value, "FORWARD") )
		*result = ACTION_FORWARD;
	else if (IS_EQUAL_STRINGS(value, "DROP") )
		*result = ACTION_DROP;
	else {
		return false;
	}
	return true;
}

static int _conf_parse_forward_default_action(cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result) {
	if ( conf_parse_forward_default_action( result, value) )
		return 0;
	cfg_error(cfg, "invalid value for option '%s': %s. Expected either FORWARD or DROP.", cfg_opt_name(opt), value);
	return -1;
}

// parse values for the input-mode option
bool conf_parse_input_mode(int *result, const char *value) {
	if (IS_EQUAL_STRINGS(value, "ONLINE") )
		*result = ONLINE_ANALYSIS;
	else if (IS_EQUAL_STRINGS(value, "OFFLINE") )
		*result = OFFLINE_ANALYSIS;
	else {
		return false;
	}
	return true;
}

static int _conf_parse_input_mode(cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result) {
	if ( conf_parse_input_mode( result, value) )
		return 0;
	cfg_error(cfg, "invalid value for option '%s': %s. Expected either ONLINE or OFFLINE.", cfg_opt_name(opt), value);
	return -1;
}

static int _conf_parse_ip_encapsulation_index(cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result) {
	if (IS_EQUAL_STRINGS(value, "FIRST") )
		*(int *) result = CONF_IP_ENCAPSULATION_INDEX_FIRST;
	else if (IS_EQUAL_STRINGS(value, "LAST") )
		*(int *) result = CONF_IP_ENCAPSULATION_INDEX_LAST;
	else{
		int val = atoi( value );
		if( val >= CONF_IP_ENCAPSULATION_INDEX_FIRST ){
			if( val <= CONF_IP_ENCAPSULATION_INDEX_LAST )
				*(int *) result = val;
			else
				*(int *) result = CONF_IP_ENCAPSULATION_INDEX_LAST;
		}else{
			cfg_error(cfg, "invalid value for option '%s': %s", cfg_opt_name(opt), value);
			return -1;
		}
	}
	DEBUG( "engine.ip-encapsulation-index = %d", *(int *) result );
	return 0;
}

static inline cfg_t *_load_cfg_from_file(const char *filename) {

	cfg_opt_t forward_packet_opts[] = {
			CFG_BOOL("enable",               true, CFGF_NONE),
			CFG_STR("output-nic",            "lo", CFGF_NONE),
			CFG_INT("snap-len",                 0, CFGF_NONE),
			CFG_INT("nb-copies",                1, CFGF_NONE),
			CFG_INT("promisc",                  0, CFGF_NONE),
			CFG_INT_CB("default",               ACTION_DROP, CFGF_NONE, _conf_parse_forward_default_action),
			CFG_STR_LIST("target-protocols", "{}", CFGF_NONE),
			CFG_STR_LIST("target-hosts",     "{}", CFGF_NONE),
			CFG_STR_LIST("target-ports",     "{}", CFGF_NONE),
			CFG_END()
		};


	cfg_opt_t engine_opts[] = {
			CFG_INT("thread-nb",                     0, CFGF_NONE),
			CFG_STR("rules-mask",                 NULL, CFGF_NONE),
			CFG_STR("exclude-rules",              NULL, CFGF_NONE),
			CFG_INT("max-instances",            100000, CFGF_NONE),
			CFG_INT_CB("ip-encapsulation-index",  CONF_IP_ENCAPSULATION_INDEX_LAST, CFGF_NONE, _conf_parse_ip_encapsulation_index),
			CFG_END()
	};

	cfg_opt_t mempool_opts[] = {
			CFG_INT("max-bytes",          2000000000, CFGF_NONE), // 2GB
			CFG_INT("max-elements",             1000, CFGF_NONE),
			CFG_INT("max-message-size",         3000, CFGF_NONE),
			CFG_INT("smp-ring-size",            1000, CFGF_NONE),
	};

	cfg_opt_t input_opts[] = {
			CFG_INT_CB("mode",  ONLINE_ANALYSIS, CFGF_NONE, _conf_parse_input_mode),
			CFG_STR("source",       "lo", CFGF_NONE),
			CFG_INT("snap-len",    65535, CFGF_NONE),
			CFG_END()
	};

	cfg_opt_t output_opts[] = {
			CFG_BOOL("enable",                 false, CFGF_NONE),
			CFG_STR("output-dir",       "./reports/", CFGF_NONE),
			CFG_INT("sample-interval",             5, CFGF_NONE),
			CFG_BOOL("report-description", true, CFGF_NONE),
			CFG_END()
	};

	cfg_opt_t dump_opts[] = {
			CFG_BOOL("enable",                 false, CFGF_NONE),
			CFG_STR("output-file","./pcap/dump.pcap", CFGF_NONE),
			CFG_END()
	};
	cfg_opt_t opts[] = {
			CFG_SEC("input",   input_opts, CFGF_NONE),
			CFG_SEC("output",  output_opts, CFGF_NONE),
			CFG_SEC("engine",  engine_opts, CFGF_NONE),
			CFG_SEC("mempool", mempool_opts, CFGF_NONE),
			CFG_SEC("forward", forward_packet_opts, CFGF_NONE),
			CFG_SEC("dump-packet", dump_opts, CFGF_NONE),


			CFG_INT("stack-type", DLT_EN10MB, CFGF_NONE),
			CFG_STR("dpdk-option", 0, CFGF_NONE),

			CFG_END()
	};

	cfg_t *cfg = cfg_init(opts, CFGF_NONE);
	switch (cfg_parse(cfg, filename)) {
	case CFG_FILE_ERROR:
		//log_write(LOG_ERR, "Error: configuration file '%s' could not be read: %s\n", filename, strerror(errno));
		cfg_free( cfg );
		return NULL;
	case CFG_SUCCESS:
		break;
	case CFG_PARSE_ERROR:
		log_write( LOG_ERR, "Error: configuration file '%s' could not be parsed.\n", filename );
		cfg_free( cfg );
		return NULL;
	}

	return cfg;
}

static inline char * _cfg_get_str( cfg_t *cfg, const char *header ){
	const char *str = cfg_getstr( cfg, header );
	if (str == NULL)
		return NULL;
	return mmt_strdup( str );
}

static inline char * _cfg_get_dir( cfg_t *cfg, const char *header ){
	const char *str = cfg_getstr( cfg, header );
	if (str == NULL)
		return NULL;
	size_t len = strlen( str );
	//ensure that a directory path is always end by '/'
	char *dir = mmt_mem_alloc_and_init_zero( len + 1 + 1 ); //+1 for '\0'; +1 for eventually '/'
	memcpy( dir, str, len + 1 ); //+1 for '\0'

	if( dir[ len - 1 ] != '/' ){
		dir[ len ]    = '/';  //append '/' if it is not there
		dir[ len + 1] = '\0'; //ensure NULL-terminated
	}
	return dir;
}

static inline cfg_t* _get_first_cfg_block( cfg_t *cfg, const char* block_name ){
	if( ! cfg_size( cfg, block_name) )
		return NULL;
	//DEBUG( "Parsing block '%s'", block_name );
	return cfg_getnsec( cfg, block_name, 0 );
}


static inline long int _cfg_getint( cfg_t *cfg, const char *ident, long int min, long int max, long int def_val ){
	long int val = cfg_getint( cfg, ident );
	if( val < min || val > max ){
		log_write( LOG_INFO, "Not expected %ld for %s. Used default value %ld.", val, ident, def_val );
		return def_val;
	}

	return val;
}

static inline input_source_conf_t * _parse_input_source( cfg_t *cfg ){
	cfg = _get_first_cfg_block( cfg, "input" );
	if( cfg == NULL )
		return NULL;

	input_source_conf_t *ret = mmt_mem_alloc_and_init_zero( sizeof( input_source_conf_t ));

	ret->input_mode   = cfg_getint(cfg, "mode");
	ret->input_source = _cfg_get_str(cfg, "source");

#ifdef DPDK_CAPTURE_MODULE
	ret->capture_mode = DPDK_CAPTURE;
#else
	ret->capture_mode = PCAP_CAPTURE;
#endif

	ret->snap_len = cfg_getint( cfg, "snap-len" );

	if( ret->snap_len == 0 )
		ret->snap_len = UINT16_MAX;

	return ret;
}

static inline output_conf_t * _parse_output( cfg_t *cfg ){
	cfg = _get_first_cfg_block( cfg, "output" );
	if( cfg == NULL )
		return NULL;

	output_conf_t *ret = mmt_mem_alloc_and_init_zero( sizeof( output_conf_t ));

	ret->is_enable   = cfg_getbool(cfg, "enable");
	ret->output_dir = _cfg_get_dir(cfg, "output-dir");
	ret->sample_interval = _cfg_getint( cfg, "sample-interval", 0, 120, 5 );
	ret->is_report_description   = cfg_getbool(cfg, "report-description");
	return ret;
}

static inline dump_packet_conf_t * _parse_dump_packet( cfg_t *cfg ){
	cfg = _get_first_cfg_block( cfg, "dump-packet" );
	if( cfg == NULL )
		return NULL;

	dump_packet_conf_t *ret = mmt_mem_alloc_and_init_zero( sizeof( dump_packet_conf_t ));

	ret->is_enable   = cfg_getbool(cfg, "enable");
	ret->output_file = _cfg_get_str(cfg, "output-file");
	return ret;
}

static inline forward_packet_conf_t *_parse_forward_packet( cfg_t *cfg ){
	cfg = _get_first_cfg_block( cfg, "forward" );
	if( cfg == NULL )
		return NULL;

	forward_packet_conf_t *ret = mmt_mem_alloc_and_init_zero( sizeof( forward_packet_conf_t ));

	ret->is_enable  = cfg_getbool( cfg, "enable" );
	ret->output_nic = _cfg_get_str(cfg, "output-nic");
	ret->snap_len = _cfg_getint( cfg, "snap-len", 0, 65535, 65535 );
	ret->nb_copies = _cfg_getint( cfg, "nb-copies", 1, UINT32_MAX, 1 );
	ret->promisc = _cfg_getint( cfg, "promisc", 0, 1, 1 );
	ret->default_action = cfg_getint( cfg, "default" );

	ret->target_size = cfg_size( cfg, "target-protocols");
	ASSERT( ret->target_size == cfg_size( cfg, "target-hosts"), "Number of elements in target-protocols and target-hosts are different");
	ASSERT( ret->target_size == cfg_size( cfg, "target-ports"), "Number of elements in target-protocols and target-ports are different");

	if( ret->target_size ){
		ret->targets = mmt_mem_alloc_and_init_zero( sizeof( forward_packet_target_conf_t ) * ret->target_size );
		int i;
		char *str;
		for( i=0; i<ret->target_size; i++) {
			//protocol
			str = cfg_getnstr(cfg, "target-protocols", i);
			if( IS_EQUAL_STRINGS(str, "SCTP") )
				ret->targets[i].protocol = FORWARD_PACKET_PROTO_SCTP;
			else if( IS_EQUAL_STRINGS(str, "UDP") )
				ret->targets[i].protocol = FORWARD_PACKET_PROTO_UDP;

			else if( IS_EQUAL_STRINGS(str, "HTTP2") )
				ret->targets[i].protocol = FORWARD_PACKET_PROTO_HTTP2;

			else
				ABORT("Does not support yet the protocol: %s", str);
			//host
			str = cfg_getnstr(cfg, "target-hosts", i);
			ret->targets[i].host = mmt_strdup( str );
			//port
			//ret->targets[i].port = cfg_getnint(cfg, "target-ports", i);
			//do not understand why ret->targets[i].port is zero
			str = cfg_getnstr(cfg, "target-ports", i);
			ret->targets[i].port = atoi( str );
		}
	}
	return ret;
}

size_t conf_parse_list( const char *string, char ***proto_lst ){
	size_t ret = 0;
	int i;
	char **lst;
	const char *str;
	const size_t len = strlen(string) +  sizeof("X={}");
	char buffer[ len ];
	ASSERT( proto_lst != NULL, "Must not be NULL" );
	//put string in form
	snprintf( buffer, len, "X={%s}", string );

	cfg_opt_t opts[] = {
			CFG_STR_LIST("X", "{}", CFGF_NONE),
			CFG_END()
	};

	cfg_t *cfg = cfg_init( opts, CFGF_NONE );
	if( cfg_parse_buf(cfg, buffer) == CFG_PARSE_ERROR )
		log_write( LOG_ERR, "Error: protocols '%s' could not be parsed.", string );
	else{
		ret = cfg_size( cfg, "X");

		lst = mmt_mem_alloc_and_init_zero( sizeof( void* ) * ret );

		for( i=0; i<ret; i++) {
			str = cfg_getnstr(cfg, "X", i);
			lst[i] = mmt_strdup( str );
		}
		*proto_lst = lst;
	}
	cfg_free( cfg );
	return ret;
}

static inline engine_conf_t *_parse_engine_block( cfg_t *cfg ){
	if( (cfg = _get_first_cfg_block( cfg, "engine")) == NULL )
		return NULL;

	engine_conf_t *ret = mmt_mem_alloc_and_init_zero( sizeof( engine_conf_t ));
	ret->threads_size = _cfg_getint( cfg, "thread-nb", 0, 128, 0 );
	ret->excluded_rules = _cfg_get_str(cfg, "exclude-rules" );
	ret->rules_mask = _cfg_get_str(cfg, "rules-mask" );
	ret->max_instances =  _cfg_getint( cfg, "max-instances", 1, UINT32_MAX, 100000 );
	ret->ip_encapsulation_index = cfg_getint(  cfg, "ip-encapsulation-index" );

	return ret;
}

static inline mempool_conf_t *_parse_mempool_block( cfg_t *cfg ){
	if( (cfg = _get_first_cfg_block( cfg, "mempool")) == NULL )
		return NULL;

	mempool_conf_t *ret = mmt_mem_alloc_and_init_zero( sizeof( mempool_conf_t ));
	ret->max_bytes = _cfg_getint( cfg, "max-bytes", 0, UINT32_MAX, 2000000000 );
	ret->max_elements = _cfg_getint( cfg, "max-elements", 0, UINT16_MAX, 1000 );
	ret->max_message_size = _cfg_getint( cfg, "max-message-size", 0, UINT16_MAX, 3000 );
	ret->smp_ring_size = _cfg_getint( cfg, "max-bytes", 0, UINT32_MAX, 1000 );

	return ret;
}

/**
 * Public API
 * @param filename
 * @return
 */
config_t* conf_load_from_file( const char* filename ){
	const char *str;
	int i;
	cfg_t *cfg = _load_cfg_from_file( filename );
	if( cfg == NULL )
		return NULL;

	config_t *conf = mmt_mem_alloc_and_init_zero( sizeof( config_t ) );

	conf->stack_type   = cfg_getint(cfg, "stack-type");
	conf->dpdk_options  = _cfg_get_str(cfg, "dpdk-option" );

	conf->input = _parse_input_source( cfg );
	conf->output = _parse_output(cfg);
	conf->dump_packet = _parse_dump_packet(cfg);

	conf->engine = _parse_engine_block( cfg );
	conf->forward = _parse_forward_packet(cfg);
	conf->mempool = _parse_mempool_block( cfg );
	cfg_free( cfg );

	return conf;
}

/**
 * Public API
 * Free all memory allocated by @load_configuration_from_file
 * @param
 */
void conf_release( config_t *conf){
	if( conf == NULL )
		return;

	int i;
	if( conf->input ){
		mmt_mem_free( conf->input->input_source );
		mmt_mem_free( conf->input );
	}
	if( conf->output ){
		mmt_mem_free( conf->output->output_dir );
		mmt_mem_free( conf->output );
	}

	if( conf->engine ){
		mmt_mem_free( conf->engine->excluded_rules );
		mmt_mem_free( conf->engine->rules_mask );
		mmt_mem_free( conf->engine );
	}

	if( conf->forward ){
		mmt_mem_free( conf->forward->output_nic );
		for( i=0; i<conf->forward->target_size; i++ ){
			mmt_mem_free( conf->forward->targets[i].host );
		}
		mmt_mem_free( conf->forward->targets );
		mmt_mem_free( conf->forward );
	}
	if( conf->dump_packet ){
		mmt_mem_free( conf->dump_packet->output_file );
		mmt_mem_free( conf->dump_packet );
	}
	mmt_mem_free( conf->mempool );
	mmt_mem_free( conf->dpdk_options );
	mmt_mem_free( conf );
}

int conf_validate( config_t *conf ){
	int ret = 0, i;
	//forward packet requires engine
	if( conf->forward->is_enable ){
		bool is_enable_engine = false;
		//when engine module is availabe inside mmt-probe => take into account its setting parameter
		ASSERT( conf->forward->nb_copies > 0, "Number of copies of packet to be sent must be greater than 0: nb-copies > 0");
		for( i=0; i<conf->forward->target_size; i++ ){
			ASSERT( conf->forward->targets[i].host != NULL, "%d-th elements of forward.target-hosts need to be initialized", (i+1) );
			ASSERT( conf->forward->targets[i].port != 0, "%d-th elements of forward.target-ports need to be initialized", (i+1) );
		}
	}

#ifdef DPDK_CAPTURE_MODULE
	if( conf->input->input_mode == OFFLINE_ANALYSIS ){
		log_write(LOG_ERR, "input.mode must be ONLINE in DPDK_CAPTURE mode");
		ret ++;
	}
#endif

	if( conf->output->is_enable ){
		//check output folder exist
		struct stat sb;

		if (stat( conf->output->output_dir, &sb) == 0 && S_ISDIR(sb.st_mode)){
			//printf("YES\n");
			//folder is existing
		} else
			ABORT( "Output folder %s does not exist. Need to update output.output-dir parameter.", conf->output->output_dir );
	}
	return ret;
}
