/*
 * plugin_engine.h
 *
 *  Created on: Oct 10, 2016
 *  Created by: Huu Nghia NGUYEN <huunghia.nguyen@montimage.com>
 *
 *  One may change #MMT_SEC_PLUGINS_REPOSITORY and #MMT_SEC_PLUGINS_REPOSITORY_OPT from Makefile
 */

#ifndef SRC_LIB_PLUGIN_ENGINE_H_
#define SRC_LIB_PLUGIN_ENGINE_H_

#ifndef MMT_SEC_PLUGINS_REPOSITORY
	#define MMT_SEC_PLUGINS_REPOSITORY "rules"
#endif

#ifndef INSTALL_DIR
	#define INSTALL_DIR "/opt/mmt/security"
#endif

#ifndef MMT_SEC_PLUGINS_REPOSITORY_OPT
	#define MMT_SEC_PLUGINS_REPOSITORY_OPT INSTALL_DIR "/rules"
#endif

#include "plugin_header.h"

/**
 * Loads all the plugins.
 * - Return
 * 	+ number of loaded rules
 */
size_t load_mmt_sec_rules( rule_info_t const *const** plugins_arr );

/**
 * Loads one plugin.
 * - Return
 * 	+ number of loaded rules
 * - Note: This function creates a new memory segment to store array of plugins.
 * 	==> user need to free it after using
 */
size_t load_mmt_sec_rule( rule_info_t const*const**plugins_arr, const char *plugin_path_name );
/**
 * Closes all loaded plugins. This function MUST only be used when the protocols corresponding
 * to the loaded plugins have been retrieved. Normally this function is used when closing the
 * library.
 */
size_t unload_mmt_sec_rules( size_t count, const uint32_t* rules_id );


#endif /* SRC_LIB_PLUGIN_ENGINE_H_ */
