/*
 * main_sec_standalone.c
 *
 *  Created on: 18 oct. 2016
 *  Created by: Huu Nghia NGUYEN <huunghia.nguyen@montimage.com>
 *
 * Standalone mmt-engine application.
 * This application can analyze (1) real-time traffic by monitoring a NIC or (2)
 * traffic saved in a pcap file. The verdicts will be printed to the current screen.
 */
#include <stdio.h>
#include <stdlib.h>
#include <pcap.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <errno.h>

#include "../lib/mmt_lib.h"
#include "../engine/dpi_message_t.h"
#include "../engine/mmt_security.h"
#include "../engine/verdict_printer.h"
#include "../engine/rule.h"
#include "../forward/forward_packet.h"

typedef struct context_struct{
	mmt_sec_handler_t *sec_handler;
	config_t *config;
	forward_packet_context_t *forward_context;
}context_t;

//Statistic
static size_t total_received_reports   = 0;
static size_t total_received_packets   = 0;
static size_t proto_atts_count        = 0;
proto_attribute_t const *const*proto_atts  = NULL;

static pcap_t *pcap = NULL;

//handler of MMT-SEC
static mmt_sec_handler_t *sec_handler  = NULL;
//handler of MMT-DPI
static mmt_handler_t *mmt_dpi_handler = NULL;
static config_t *config = NULL;
static context_t context = {0};

#define DEFAULT_CONFIG_FILE     "./mmt-5greplay.conf"
void usage(const char * prg_name) {
	printf("%s [<Option>]\n", prg_name);
	printf("Option:\n");
	printf("\t-v               : Print version information, then exits.\n");
	printf("\t-c <config file> : Gives the path to the configuration file (default: %s).\n",
				DEFAULT_CONFIG_FILE);
	printf("\t-t <trace file>  : Gives the trace file for offline analyse.\n");
	printf("\t-i <interface>   : Gives the interface name for live traffic analysis.\n");
	printf("\t-X attr=value    : Override configuration attributes.\n");
	printf("\t                    For example \"-X output.enable=true -Xoutput.output-dir=/tmp/\" will enable output to file and change output directory to /tmp.\n");
	printf("\t                    This parameter can appear several times.\n");
	printf("\t-x               : Prints list of configuration attributes being able to be used with -X, then exits.\n");
	printf("\t-h               : Prints this help, then exits.\n");
}

/**
 * Free a string pointer before clone data to it
 */
static inline void _override_string_conf( char **conf, const char*new_val ){
	mmt_mem_free( *conf );
	*conf = mmt_strdup( new_val );
}


static inline config_t *_parse_options(int argc, char ** argv ) {
	int opt;
	const char *config_file = DEFAULT_CONFIG_FILE;
	config_t *conf = NULL;
	const char *options = "t:i:c:X:xh";

	//to get config
	extern char *optarg;
	extern int optind;

	char *string_att, *string_val;

	bool is_user_gives_conf_file = false;

	while ((opt = getopt(argc, argv, options)) != EOF) {
		switch (opt) {
		case 'c':
			config_file = optarg;
			break;
		case 'x':
			conf_print_identities_list();
			exit( EXIT_SUCCESS );
			break;
		case 'X':
		case 't':
		case 'i':
			break;
		case 'h':
		default:
			usage(argv[0]);
			exit( EXIT_SUCCESS );

		}
	}

	conf = conf_load_from_file( config_file );
	if( conf == NULL ){
		log_write_dual(LOG_ERR, "Cannot read configuration file from \"%s\"\n", config_file );
		exit( EXIT_FAILURE );
	}

	//reset getopt function
	optind = 0;

	//override some options inside the configuration
	while ((opt = getopt(argc, argv, options)) != EOF) {
		switch (opt) {
		//trace file
		case 't':
			_override_string_conf( &conf->input->input_source, optarg );
			//switch to offline mode
			conf->input->input_mode = OFFLINE_ANALYSIS;
			break;
			//input interface
		case 'i':
			_override_string_conf( &conf->input->input_source, optarg );
			//switch to online mode
			conf->input->input_mode = ONLINE_ANALYSIS;
			break;

		case 'X':
			//example: -X file-output.enable=true
			//we will separate the phrase "file-output.enable=true" into 2
			// to expect:
			//   string_att = "file-output.enable"
			//   string_val = "true"
			string_att = optarg;
			string_val = optarg;
			while( *string_val != '\0' ){
				//separated by = character
				if( *string_val == '=' ){
					*string_val = '\0'; //NULL ended for attribute
					//jump to the part after = character
					string_val ++;
					break;
				}
				string_val ++;
			}
			//not found = character
			if( *string_val == '\0' )
				log_write( LOG_WARNING, "Input parameter '%s' is not well-formatted (must be in format parameter=value). Ignored it.", string_att );

			switch( conf_override_element(conf, string_att, string_val) ){
			case 0:
				//log_write( LOG_INFO, "Overridden value of configuration parameter '%s' by '%s'", string_att, string_val );
				break;
			case -1:
				log_write_dual(LOG_ERR, "Unknown parameter identity %s\n", string_att );
				exit( EXIT_FAILURE );
			}

		}
	}

	if( conf_validate(conf) ){
		abort();
	}

	return conf;
}


/**
 * A function to be called when a rule is validated
 * Note: this function can be called from one or many different threads,
 *       ==> be carefully when using static or shared variables inside it
 */
static void _print_security_verdict(
		const rule_info_t *rule,        //rule being validated
		enum verdict_type verdict,      //DETECTED, NOT_RESPECTED
		uint64_t timestamp,             //moment (by time) the rule is validated
		uint64_t counter,               //moment (by order of packet) the rule is validated
		const mmt_array_t * trace,      //historic messages that validates the rule
		void *user_data                 //#user-data being given in register_security
){
	forward_packet_context_t *context = (forward_packet_context_t *) user_data;
	char message[10000];
	size_t len;

	//Special processing when the rule is FORWARD
	if( rule->type_id == RULE_TYPE_FORWARD && context ){
		//mark that there exists a FORWARD rule that is satisfied,
		//  thus do not need to peform default action (forward/drop) on the current packet
		//  because this action is decided by user in the satisfied rule
		forward_packet_mark_being_satisfied( context );
	}

	if( config->output->is_enable ){
		const char *exec_trace  = mmt_convert_execution_trace_to_json_string( trace, rule );
		len = snprintf( message, sizeof( message ), "%ld,%"PRIu32",\"%s\",\"%s\",\"%s\",%s",
				time( NULL ),
				rule->id,
				verdict_type_string[verdict],
				rule->type_string,
				(config->output->is_report_description? rule->description : ""),
				exec_trace );
		message[len] = '\0';
		verdict_printer_send( message );
	}
}

/**
 * Convert a pcap packet to a message being understandable by mmt-engine.
 * The function returns NULL if the packet contains no interested information.
 * Otherwise it creates a new memory segment to store the result message. One need
 * to use #free_message_t to free the message.
 */
static inline message_t* _get_packet_info( const ipacket_t *pkt ){
	int i;
	void *data;
	int type;

	message_t *msg = create_message_t();
	msg->timestamp = mmt_sec_encode_timeval( &pkt->p_hdr->ts );
	msg->counter   = pkt->packet_id;
	msg->flow_id   = get_session_id_from_packet( pkt );
	//get a list of proto/attributes being used by mmt-engine
	for( i=0; i<proto_atts_count; i++ )
		dpi_message_set_data( pkt, proto_atts[i]->dpi_type, msg, proto_atts[i]->proto_id, proto_atts[i]->att_id );

	if( likely( msg->elements_count ))
		return msg;

	//need to free #msg when the packet contains no-interested information
	free_message_t( msg );
	return NULL;
}


/**
 * Register an attribute of a protocol to MMT-DPI. They are given by their IDs
 * @param proto_id
 * @param att_id
 * @param verbose
 * @return true if it is registered successfully
 * 		   false if it has been registered or it can not be registered
 */
static inline bool _register_proto_att_to_mmt_dpi( uint32_t proto_id, uint32_t att_id, bool verbose ){
	//is it registered?
	if( is_registered_attribute( mmt_dpi_handler, proto_id, att_id ))
		return 0;
	if( register_extraction_attribute( mmt_dpi_handler, proto_id, att_id ) ){
		log_write(LOG_INFO, "Registered attribute to extract: %"PRIu32".%"PRIu32, proto_id, att_id );
		return 1;
	}
	return 0;
}

/**
 * update of list of unique att_protos and register them to MMT-DPI
 * @return number of att_protos being registered
 */
static inline size_t _update_and_register_protocols_attributes_to_extract( bool verbose ){
	int i;
	size_t ret = 0;
	proto_atts_count = mmt_sec_get_unique_protocol_attributes( & proto_atts );

	for( i=0; i<proto_atts_count; i++ ){

		ret += _register_proto_att_to_mmt_dpi( proto_atts[i]->proto_id, proto_atts[i]->att_id, verbose  );

		//tcp.p_payload => need payload_len
		if( proto_atts[i]->proto_id == PROTO_TCP && proto_atts[i]->att_id == PROTO_PAYLOAD ){
			//tcp.payload_len
			ret += _register_proto_att_to_mmt_dpi( PROTO_TCP, TCP_PAYLOAD_LEN, verbose );
		}else if( proto_atts[i]->proto_id == PROTO_IP && proto_atts[i]->att_id == IP_OPTS){
			ret += _register_proto_att_to_mmt_dpi( PROTO_IP, IP_HEADER_LEN, verbose );
		}
	}
	return ret;
}

#ifdef MODULE_ADD_OR_RM_RULES_RUNTIME
static inline void _print_add_rm_rules_instruction(){
	log_write( LOG_INFO,"During runtime, user can add or remove some rules using the following commands:\n%s\n%s",
		" - to add new rules: add rule_mask, for example: add (1:1-3)(2:4-6)",
		" - to remove existing rules: rm rule_range, for example: rm  1-3");
}

/**
 * Add rules to process and update DPI to extract the corresponding protos/atts
 * @param rules_mask
 * @return number of rules being added
 */
static inline size_t _add_rules( const char* rules_mask ){
	size_t ret = mmt_sec_add_rules(rules_mask);
	//no new rules being added
	if( ret == 0 )
		return ret;

	//register the new proto_atts if need
	size_t count = _update_and_register_protocols_attributes_to_extract( false );
	DEBUG( "Registered %zu new proto_atts", count );

	return ret;
}


static inline size_t _remove_rules( size_t rules_count, const uint32_t *rules_ids_array ){
	proto_attribute_t const*const* old_proto_atts;
	proto_attribute_t const*const* new_proto_atts;
	size_t old_proto_atts_count, new_proto_atts_count;
	size_t i, j;

	old_proto_atts_count = mmt_sec_get_unique_protocol_attributes( & old_proto_atts );

	size_t ret = mmt_sec_remove_rules( rules_count, rules_ids_array );
	//no rules being removed ???
	if( ret == 0 )
		return ret;

	new_proto_atts_count = mmt_sec_get_unique_protocol_attributes( & new_proto_atts );

	//set of proto_atts does not change after removing some rules => donot need to unregister any proto_att
	if( old_proto_atts_count == new_proto_atts_count )
		return ret;

	//unregister the att_protos of rules being removed
	//for each old protol_att
	for( i=0; i<old_proto_atts_count; i++ ){
		for( j=0; j<new_proto_atts_count; j++ )
			if( old_proto_atts[i]->proto_id == new_proto_atts[i]->proto_id &&
				 old_proto_atts[i]->att_id == new_proto_atts[i]->att_id )
				break; //this proto_att is still needed
		//
		if( j <= new_proto_atts_count )
			continue;
		//unregister this old proto_att
		unregister_extraction_attribute(mmt_dpi_handler, old_proto_atts[i]->proto_id, old_proto_atts[i]->att_id );
		DEBUG("Unregistered from mmt-dpi: %"PRIu32".%"PRIu32" (%s,%s)",
				old_proto_atts[i]->proto_id, old_proto_atts[i]->att_id,
				old_proto_atts[i]->proto, old_proto_atts[i]->att );
	}
	return ret;
}

/**
 * This has to be called before any stdin input function.
 * When I used std::cin before using this function, it never returned true again.
 * @return true if user press some keys ended by Enter
 */
static inline bool _is_user_press_keys(){
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds); //add stdin to fsd, STDIN_FILENO is 0
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    return ( FD_ISSET(STDIN_FILENO, &fds) != 0 );
}

void _add_or_remove_rules_if_need(){
	const int len = 1000;

	char buffer[ 1000 ], *c;
	size_t count;
	uint32_t *rules_id_to_rm_set;
	//if user does not press any keys
	if( _is_user_press_keys() == false )
		return;

	//get user's string
	if( !fgets( buffer, len, stdin) )
		return;

	//as fgets add EOF or EOL at the end of buffer => we need to remove these special characters
	c = buffer;
	while( *c != '\0' ){
		if( *c == EOF || *c == '\n' ){
			*c = '\0';
			break;
		}
		c++;
	}


	if( buffer[0] == '\0' )
		return;

	//add xxxx
	if( buffer[0] == 'a' && buffer[1] == 'd' && buffer[2] == 'd'  && buffer[3] == ' ' ){
		log_write( LOG_INFO, "Added totally %zu rule(s)", _add_rules( &buffer[4] ));
		return;
	}else //rm xxx
		if( buffer[0] == 'r' && buffer[1] == 'm'  && buffer[2] == ' ' ){
			count = expand_number_range( &buffer[3], &rules_id_to_rm_set );
			if( count > 0 ){
				count = _remove_rules( count, rules_id_to_rm_set);
			}
			log_write( LOG_INFO, "Removed totally %zu rule(s)", count);

			//free memory allocated by expand_number_range
			mmt_mem_free( rules_id_to_rm_set );
			return;
	}

	log_write( LOG_WARNING,"Unknown command \"%s\"", buffer );
	_print_add_rm_rules_instruction();
}
/* Returns an integer in the range [1, n].
 *
 * Uses rand(), and so is affected-by/affects the same seed.
 */
static inline int _rand_int(unsigned int n) {
  if ((n - 1) == RAND_MAX) {
    return rand();
  } else {
    // Chop off all of the values that would cause skew...
    long end = RAND_MAX / n; // truncate skew
    end *= n;

    // ... and ignore results from rand() that fall above that limit.
    // (Worst case the loop condition should succeed 50% of the time,
    // so we can expect to bail out of this loop pretty quickly.)
    int r;
    while ((r = rand()) >= end);

    return r % n + 1;
  }
}

static inline bool _rand_bool(){
	return (_rand_int( 10 ) > 5);
}
#else
#define _add_or_remove_rules_if_need()
#define _print_add_rm_rules_instruction()
#endif



/**
 * This function is called by mmt-dpi for each incoming packet containing registered proto/att.
 * It gets interested information from the #ipkacet to a message then sends the
 * message to mmt-engine.
 */
static int _packet_handle( const ipacket_t *ipacket, void *args ) {
	uint32_t rm_rules_arr[50];
	char string[500], *ch = string;
	int i;
	int ret = 0;

	context_t *context = (context_t *) args;
	MUST_NOT_OCCUR( context == NULL, "args parameter must not be NULL"); //this must not happen

	if( context->config->forward->is_enable )
		forward_packet_on_receiving_packet_before_rule_processing( ipacket, context->forward_context );

	uint64_t flow_id = get_session_id_from_packet( ipacket );

	message_t *msg = _get_packet_info( ipacket );

	total_received_packets ++;

	//if there is no interested information
	//TODO: to check if we still need to send timestamp/counter to mmt-sec?
	if( unlikely( msg == NULL )){
		goto __finish_security;
		ret = 1;
	}

	mmt_sec_process( context->sec_handler, msg );

//TODO: remve this block
//#ifdef MODULE_ADD_OR_RM_RULES_RUNTIME
//	if( total_received_reports == 1000 ){
//		DEBUG("Add %zu rules", _add_rules("(1:33,32,34)"));
//		//need to add/rm or not?
//		if( _rand_bool() ){
//			printf("\n%zu\n", total_received_reports );
//			//add or rm rules?
//			if( _rand_bool() ){
//				//rm random rules ID
//				int nb_rules_to_rm = _rand_int( 5 );
//				for( i=0; i<nb_rules_to_rm; i++ )
//					rm_rules_arr[i] = _rand_int( 50 );
//				mmt_sec_remove_rules( nb_rules_to_rm, rm_rules_arr );
//			}else{
//				//add
//				int nb_rules_to_add = _rand_int( 5 );
//				ch = string;
//				ch += sprintf(string, "(%d:", _rand_int(9) );
//				for( i=0; i<nb_rules_to_add; i++ )
//					ch += sprintf(ch, "%d,", _rand_int( 50 ) );
//				*ch = '\0';
//				*(ch - 1) = ')';
//
//				_add_rules( string );
//
//			}
//		}
//	}
//#endif

	total_received_reports ++;

	//when forwarding packet is enable
	// we need to call this function to forward the current packet if it is not satisfied by any rule
	__finish_security:
	if( context->config->forward->is_enable )
		forward_packet_on_receiving_packet_after_rule_processing( ipacket, context->forward_context );


	return ret;
}

void live_capture_callback( u_char *user, const struct pcap_pkthdr *p_pkthdr, const u_char *data ){
	mmt_handler_t *mmt = (mmt_handler_t*)user;
	struct pkthdr header;

	//allow user to add/rm rules
	_add_or_remove_rules_if_need();

	header.ts     = p_pkthdr->ts;
	header.caplen = p_pkthdr->caplen;
	header.len    = p_pkthdr->len;
	if (!packet_process( mmt, &header, data )) {
		log_write(LOG_ERR, "Packet data extraction failure.\n");
	}
	//printf("."); fflush( stdout );
}


static inline void termination(){
	struct pcap_stat pcs; /* packet capture filter stats */
	size_t alerts_count;

	if( pcap )
		pcap_breakloop( pcap );

	alerts_count = mmt_sec_unregister( sec_handler );

	memset( &pcs, 0, sizeof(pcs) );
	if (pcap && pcap_stats(pcap, &pcs) < 0) {
//		(void) fprintf(stderr, "pcap_stats: %s\n", pcap_geterr( pcap ));//Statistics aren't available from savefiles
	}else{
		(void) fprintf(stderr, "\n%12d packets received by filter\n", pcs.ps_recv);
		(void) fprintf(stderr, "%12d packets dropped by interface\n", pcs.ps_ifdrop);
		(void) fprintf(stderr, "%12d packets dropped by kernel (%3.2f%%)\n", pcs.ps_drop,
				pcs.ps_recv==0? 0 : (pcs.ps_drop * 100.0 / pcs.ps_recv));
		fflush(stderr);
	}

	fprintf(stderr, "%12zu packets received\n", total_received_packets );
	fprintf(stderr, "%12zu messages received\n", total_received_reports );
	fprintf(stderr, "%12zu alerts generated\n", alerts_count );

	if( pcap )
		pcap_close( pcap );

	if( config->output->is_enable )
		verdict_printer_free();

	mmt_sec_close();   // close mmt_security
	if( mmt_dpi_handler ){
		close_extraction();// close mmt_dpi
		mmt_dpi_handler = NULL;
	}
	conf_release( config );
	forward_packet_release(context.forward_context);
}

void signal_handler_seg(int signal_type) {
	log_write( LOG_ERR, "Interrupted by signal %d", signal_type );
	log_execution_trace();
	exit( signal_type );
}

void signal_handler(int signal_type) {
	static volatile int times_counter = 0;

	if( times_counter >= 1 ) exit( signal_type );
	times_counter ++;

	log_write( LOG_ERR, "Interrupted by signal %d", signal_type );

	if( signal_type == SIGINT ){
		log_write( LOG_ERR,"Releasing resource ... (press Ctrl+c again to exit immediately)");
		signal(SIGINT, signal_handler);
		termination();
	}
	exit( signal_type );
}


//do nothing here
// we handle the error in the packet injector (HTTP2 or SCTP, etc)
void sigpipe_signal_handler( int signal_type ){}

void register_signals(){
#ifndef DEBUG_MODE
	signal(SIGSEGV, signal_handler_seg );
#endif
	signal(SIGINT,  signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGABRT, signal_handler);

	//SIGPIPE is raised when we are trying to write into a closed socket
	// => we need to capture this signal to avoid quiting 5Greplay
	signal(SIGPIPE, sigpipe_signal_handler);
}

int replay(int argc, char** argv) {
	char mmt_errbuf[1024];

	const unsigned char *data;
	struct pcap_pkthdr p_pkthdr;
	char errbuf[1024];
	int ret;
	struct pkthdr header;
	size_t i, j, size;
	uint16_t *rules_id_filter = NULL;


#define MAX_ENV_STRING_LEN 1024
	char environment[MAX_ENV_STRING_LEN];

	register_signals();

	config = _parse_options(argc, argv);

	//export some environments variables to expose configuration parameters
	// so that the embedded functions in the rules can access to these parameters
	snprintf( environment, MAX_ENV_STRING_LEN, "MMT_5GREPLAY_NB_COPIES=%u", config->forward->nb_copies );
	snprintf( environment, MAX_ENV_STRING_LEN, "MMT_5GREPLAY_HTTP2_NB_COPIES=%u", config->forward->nb_copies );
	putenv( environment );

	ret = mmt_sec_init( config->engine->excluded_rules );
	if( ret != 0 ){
		conf_release( config );
		exit( EXIT_FAILURE );
	}

	if( config->output->is_enable )
		verdict_printer_init( config->output->output_dir, config->output->sample_interval );

	//init mmt_dpi extraction
	init_extraction();

	//Initialize dpi handler
	mmt_dpi_handler = mmt_init_handler( config->stack_type, 0, mmt_errbuf);
	if (!mmt_dpi_handler) { /* pcap error ? */
		fprintf(stderr, "MMT handler init failed for the following reason: %s\n", mmt_errbuf);
		return EXIT_FAILURE;
	}

	forward_packet_context_t *forward_context = forward_packet_alloc( config,  mmt_dpi_handler );

	sec_handler =  mmt_sec_register( config->engine->threads_size,
			NULL,  //core_id is NULL to allow OS arbitrarily arranging security threads on logical cores
			config->engine->rules_mask,
			true,
			_print_security_verdict,
			forward_context );

	_update_and_register_protocols_attributes_to_extract( true );

	context.config = config;
	context.forward_context = forward_context;
	context.sec_handler = sec_handler;

	//Register a packet handler, it will be called for every processed packet
	register_packet_handler(mmt_dpi_handler, 1, _packet_handle, &context );

	_print_add_rm_rules_instruction();

	if( config->input->input_mode == OFFLINE_ANALYSIS ) {
		log_write( LOG_INFO,"Analyzing pcap file %s", config->input->input_source );
		pcap = pcap_open_offline(config->input->input_source, errbuf); // open offline trace
		if (!pcap) { /* pcap error ? */
			ABORT("pcap_open failed for the following reason: %s\n", errbuf);
		}

		while ((data = pcap_next(pcap, &p_pkthdr)) ) {
			//allow user to add/rm rules
			_add_or_remove_rules_if_need();


			header.ts     = p_pkthdr.ts;
			header.caplen = p_pkthdr.caplen;
			header.len    = p_pkthdr.len;
			if (!packet_process(mmt_dpi_handler, &header, data))
				log_write( LOG_ERR, "Packet data extraction failure.\n");
		}

	} else {
		log_write( LOG_INFO,"Listening on interface %s", config->input->input_source );

		pcap = pcap_create( config->input->input_source, errbuf);
		if (pcap == NULL)
			ABORT("Couldn't open device %s\n", errbuf);

		pcap_set_snaplen(pcap, config->input->snap_len);
		pcap_set_promisc(pcap, 1);
		pcap_set_timeout(pcap, 0);
		pcap_set_buffer_size(pcap, 100*1000*1000);
		pcap_activate(pcap);

		(void)pcap_loop( pcap, -1, &live_capture_callback, (u_char*)mmt_dpi_handler );
	}

	termination();
	return EXIT_SUCCESS;
}


/**
 * Public API
 */
uint32_t conf_get_number_value( config_identity_t id ){
	switch( id ){
	case CONF_ATT__ENGINE__MAX_INSTANCES:
		return config->engine->max_instances;
	case CONF_ATT__MEMPOOL__MAX_BYTES:
		return config->mempool->max_bytes;
	case CONF_ATT__MEMPOOL__MAX_ELEMENTS:
		return config->mempool->max_elements;
	case CONF_ATT__MEMPOOL__MAX_MESSAGE_SIZE:
		return config->mempool->max_message_size;
	case CONF_ATT__MEMPOOL__SMP_RING_SIZE:
		return config->mempool->smp_ring_size;
	default:
		ABORT("Does not support yet the config identity %d\n", id);
	}
	return 0;
}
