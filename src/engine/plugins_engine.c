/*
 * plugins_engine.c
 *
 *  Created on: Oct 10, 2016
 *  Created by: Huu Nghia NGUYEN <huunghia.nguyen@montimage.com>
 */
#include <dirent.h>
#include <dlfcn.h>
#include <stdlib.h>

#include "plugins_engine.h"
#include "../lib/mmt_lib.h"

typedef size_t ( *mmt_sec_get_plugin_info_fn_t )( const rule_info_t ** );
typedef void (*void_fn_t)();

typedef struct plugin_struct{
	char *path;                //full path of this plugin
	void *dl_lib;			  //pointer given by dl_open

	const rule_info_t **rules; //array of rules. This array contains only the rules that have different ids with the rules existing
	const rule_info_t **original_rules; //original array of rules

	uint16_t rules_count;    //number of rules in this plugin
	uint16_t original_rules_count;

	void_fn_t on_unload;
}plugin_t;


//TODO: this limit 50K files .so and 50K rules
#define MAX_PLUGIN_COUNT 50000
#define MAX_RULES_COUNT  50000

static plugin_t plugins[MAX_PLUGIN_COUNT];
static const rule_info_t *rules[ MAX_RULES_COUNT ];

static uint32_t plugins_count = 0; //number of plugins (number of .so files)
static uint32_t rules_count   = 0; //number of rules inside the plugins


static int _load_filter( const struct dirent *entry ){
	char *ext = strrchr( entry->d_name, '.' );
	return( ext && !strcmp( ext, ".so" ));
}

static inline bool _find_plugin_has_rule_id( uint32_t rule_id, size_t *plugin_index, size_t *rule_index ){
	size_t i, j;
	for( i=0; i<plugins_count; i++ )
		for( j=0; j<plugins[i].rules_count; j++ )
			if( plugins[i].rules[j]->id == rule_id ){
				*plugin_index = i;
				*rule_index   = j;
				return YES;
			}
	return NO;
}


/**
 * Check if a plugin was loaded
 * @param lib_path
 * @return - index of plugin in #plugins set if it was loaded
 *         - total number of existing plugins, otherwise
 */
static inline int _find_plugin_by_name( const char *lib_path ){
	int i;
	for( i=0; i<plugins_count; i++ )
		//found a path existing in #dl_libs_path
		if( strcmp( lib_path, plugins[i].path ) == 0 )
			return i;

	return plugins_count;
}


static inline bool _load_plugin_by_functions(
		plugin_t *plugin,
		mmt_sec_get_plugin_info_fn_t mmt_sec_get_plugin_info,
		void_fn_t on_load,
		void_fn_t on_unload ){

	rule_info_t const* tmp_array;
	rule_info_t const** original_rules_array;
	rule_info_t const** rules_array;

	size_t size, i, index = 0, p_index, r_index;

	size = mmt_sec_get_plugin_info( &tmp_array );
	if( size == 0 )
		return false;

	uint32_t required_plugin = mmt_sec_get_required_plugin_version_number();
	original_rules_array = malloc( sizeof( rule_info_t *) * size );

	//filter out the rules that are not satisfied the requirement version
	for( i=0; i<size; i++ ){
		if( tmp_array[i].version->index < required_plugin )
			log_write( LOG_WARNING, "Ignored rule %d as it is not up to date.\nRule description: %s",
					tmp_array[i].id,
					tmp_array[i].description );
		else
			original_rules_array[ index++ ] = &tmp_array[i];
	}

	plugin->original_rules       = original_rules_array;
	plugin->original_rules_count = index;
	plugin->rules                = malloc( sizeof( void*) *  plugin->original_rules_count );
	ASSERT( plugin->rules != NULL, "Not enough memory");

	//filter out the duplicated rules
	//add distinct rules from #original_rules to #rules
	for( i=0; i<plugin->original_rules_count; i++ ){
		//not exist ?
		if( ! _find_plugin_has_rule_id( plugin->original_rules[i]->id, &p_index, &r_index )){
			ASSERT( rules_count < MAX_RULES_COUNT, "Support maximally %d rules", MAX_RULES_COUNT );

			//this rule is fresh

			//set of rules of this plugin
			plugin->rules[ plugin->rules_count ++ ] = plugin->original_rules[i];

			//a set of total rules
			rules[ rules_count ++ ] = plugin->original_rules[i];
		}else
			log_write( LOG_WARNING, "Ignore duplicated rule id %d (%s)",
					plugin->original_rules[i]->id,
					plugin->original_rules[i]->description );
	}

	if( plugin->rules_count == 0 ){
		free( plugin->original_rules );
		free( plugin->rules );
		return false;
	}

	//remember unload function to call it when finish
	plugin->on_unload = on_unload;

	//we call on_load here as the plugin has been sucessfully loaded
	if( on_load != NULL )
		on_load();
	//else
	//	DEBUG("Not found on_load on %s", plugin_path_name);
	return true;
}


size_t mmt_sec_load_plugin(
		size_t ( *mmt_sec_get_plugin_info ) ( const rule_info_t ** ),
		void (*on_load)(),
		void (*on_unload)() ){

	if( mmt_sec_get_plugin_info == NULL )
		return 0;

	ASSERT( plugins_count < MAX_PLUGIN_COUNT, "Support maximally %d plugins", MAX_PLUGIN_COUNT );

	plugin_t *plugin = &plugins[ plugins_count ];

	if( _load_plugin_by_functions(plugin, mmt_sec_get_plugin_info, on_load, on_unload )){
		plugin->path      = NULL;
		plugin->dl_lib    = NULL;

		plugins_count ++;
		return plugin->rules_count;
	}

	return 0;
}

static inline size_t _load_plugin_by_path( const char *plugin_path_name ){
	__check_null( plugin_path_name, 0 );

	//this .so is opening
	//this happens when #load_mmt_sec_rules is called many times
	int plugin_index = _find_plugin_by_name( plugin_path_name );
	if( plugin_index < plugins_count )
		return plugins[ plugin_index ].original_rules_count ;

	ASSERT( plugins_count < MAX_PLUGIN_COUNT, "Support maximally %d plugins", MAX_PLUGIN_COUNT );

	void *lib = dlopen( plugin_path_name, RTLD_NOW );

	ASSERT( lib != NULL, "Cannot open library: %s.\n%s", plugin_path_name, dlerror() );

	mmt_sec_get_plugin_info_fn_t mmt_sec_get_plugin_info = dlsym ( lib, "mmt_sec_get_plugin_info" );

	ASSERT( mmt_sec_get_plugin_info != NULL, "File %s is incorrect!", plugin_path_name );

	void_fn_t on_load   = dlsym( lib, "on_load" );
	void_fn_t on_unload = dlsym( lib, "on_unload" );

	plugin_t *plugin = &plugins[ plugins_count ];

	if( _load_plugin_by_functions(plugin, mmt_sec_get_plugin_info, on_load, on_unload )){
		plugin->path      = strdup( plugin_path_name );
		plugin->dl_lib    = lib;

		plugins_count ++;
		return plugin->rules_count;
	}
	return 0;
}

//#define STATIC_RULES_SUFFIX_LIST SUFFIX(1_ssh) SUFFIX(10_http_port) SUFFIX(100_test_TCP) SUFFIX(101_SQI_session_payload) SUFFIX(11_ip_size) SUFFIX(12_http_uri) SUFFIX(13_datainSYN) SUFFIX(14_illegal_port) SUFFIX(15_nikto)

#ifdef STATIC_RULES_SUFFIX_LIST
#define __STATIC_RULES

/* At compiling moment, if we got a list of plugins that will be embedded into libmmt_security,
 * STATIC_RULES_SUFFIX_LIST is list of suffix of rule plugins.
 *
 * We use this list to declare 3 kinds of header functions that have been implemented by the rule plugins:
 * A suffix xx is created from the name a plugin by replacing non-alphanumeric characters by underscores.
 * For example: the rule plugin "1.ssh.xml" will give a suffix "1_ssh".
 *
 * void on_load_xx();
 * void on_unload_xx();
 * size_t mmt_sec_get_plugin_info_xx( const rule_info_t ** );
 */
#define SUFFIX(xx)           \
	void on_load_ ##xx();    \
	void on_unload_ ##xx();  \
	size_t mmt_sec_get_plugin_info_ ##xx( const rule_info_t ** );

//we declare the header list here
STATIC_RULES_SUFFIX_LIST

//redefine SUFFIX macro to fire the functions declared above
#undef SUFFIX
#define SUFFIX(xx) mmt_sec_load_plugin( mmt_sec_get_plugin_info_ ##xx, on_load_ ##xx, on_unload_ ##xx);

void _preload_static_rules(){
	STATIC_RULES_SUFFIX_LIST
}

#endif

size_t load_mmt_sec_rules( rule_info_t const*const**ret_array ){
	size_t i, j, k;
	char path[ 1001 ];
	struct dirent **entries, *entry;
	const char *plugin_folder;

	int n;

	plugin_folder = MMT_SEC_PLUGINS_REPOSITORY;
	n = scandir( plugin_folder, &entries, _load_filter, alphasort );
	if( n < 0 ) {
		/* can't read PLUGINS_REPOSITORY -> just ignore and return success
		 * (the directory may not exist or may be inaccessible, that's ok)
		 * note: no entries were allocated at this point, no need for free().
		 */
		plugin_folder = MMT_SEC_PLUGINS_REPOSITORY_OPT;
		n = scandir( plugin_folder, &entries, _load_filter, alphasort );
	}

	if( n >= 0 ){
		for( i = 0 ; i < n ; ++i ) {
			entry = entries[i];
			(void) snprintf( path, 1000, "%s/%s", plugin_folder, entry->d_name );

			//load plugin
			_load_plugin_by_path( path );

			free( entry );
		}
		free( entries );
	}

	//we give higher priority to the plugins being loaded dynamically
	// we then load statically plugins that have been embedded into the program
#ifdef __STATIC_RULES
	_preload_static_rules();
#endif

	*ret_array = rules;

	return rules_count;
}


/**
 * PUBLIC API
 */
size_t load_mmt_sec_rule( rule_info_t const*const**rules_array, const char *plugin_path_name ){
	void *lib = dlopen( plugin_path_name, RTLD_NOW );

	ASSERT( lib != NULL, "Cannot open library: %s.\n%s", plugin_path_name, dlerror() );

	mmt_sec_get_plugin_info_fn_t mmt_sec_get_plugin_info = dlsym ( lib, "mmt_sec_get_plugin_info" );

	rule_info_t const* tmp_array;

	size_t size, i;

	size = mmt_sec_get_plugin_info( &tmp_array );

	const rule_info_t **rules = malloc( sizeof( void*) *  size );
	ASSERT( rules != NULL, "Not enough memory");

	for( i=0; i<size; i++ ){
		//set of rules of this plugin
		rules[ i ] = &tmp_array[i];
	}
	*rules_array = rules;
	return size;
}

/**
 * Close a plugin
 * @param plugin
 */
static inline int _close_plugin( plugin_t *plugin ){
	int ret = 0;
	//execute on_unload function inside the plugin if need
	if( plugin->on_unload != NULL )
		plugin->on_unload();
	//else
	//	DEBUG("Not found on_unload");

	if( plugin->dl_lib ){
		ret = dlclose( plugin->dl_lib );
		if( ret != 0 )
			log_write( LOG_WARNING,"Cannot close properly plugin: %s", plugin->path );
	}

	//free memory created by strdup
	free( plugin->path );
	free( plugin->rules );
	free( plugin->original_rules );

	return ret;
}

/**
 * unload a rule
 *
 */
static inline bool _unload_mmt_sec_rule( uint32_t rule_id ){
	size_t rule_index, plugin_index;
	__check_bool( (plugins_count == 0), NO );

	//does not found any rule having id = rule_id
	if( _find_plugin_has_rule_id(rule_id, &plugin_index, &rule_index) == NO )
		return NO;

	//there is only one rule in this plugin => remove the plugin
	if( plugins[ plugin_index ].rules_count <= 1 ){
		//first close the plugin
		_close_plugin( &plugins[plugin_index] );

		//then remove it from array by overriding it by the last element in the array
		plugins_count --;
		plugins[ plugin_index ] = plugins[ plugins_count ];

		return YES;
	}

	plugins[ plugin_index ].rules_count --;
	//remove this rule by replacing it by the last rule then remove the last rule
	plugins[ plugin_index ].rules[ rule_index ] = plugins[ plugin_index ].rules[ plugins[ plugin_index ].rules_count ];
	return YES;
}

/**
 * Public API
 */
size_t unload_mmt_sec_rules( size_t count, const uint32_t* rules_id ){
	size_t ret = 0, i;
	for( i=0; i<count; i++ )
		if( _unload_mmt_sec_rule(rules_id[i]) == YES )
			ret ++;
	return ret;
}

//call when exiting application
__attribute__((destructor))
void unload_mmt_sec_all_rules() {
	int i, ret = 0;
	for( i=0; i<plugins_count; i++ )
		ret |= _close_plugin( &plugins[i] );

	if( ret != 0 )
		log_write( LOG_WARNING,"Cannot close properly mmt-security .so rules");

	plugins_count = 0;
}
