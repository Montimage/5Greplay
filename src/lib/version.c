/*
 * version.c
 *
 *  Created on: Apr 18, 2017
 *  Created by: Huu Nghia NGUYEN <huunghia.nguyen@me.com>
 */
#include <stdlib.h>
#include "version.h"

//VERSION_NUMBER is given by Makefile
#ifndef VERSION_NUMBER
  #error("Not found VERSION_NUMBER")
#endif

//update this number when structure of plugin has been changed
#define REQUIRE_PLUGIN "0.0.1"


#define __NOW__   __DATE__ " " __TIME__

#ifdef GIT_VERSION
	//GIT_VERSION is given by Makefile
	#define MMT_SEC_VERSION  VERSION_NUMBER " (" GIT_VERSION " - " __NOW__ ")"
#else
	#define MMT_SEC_VERSION VERSION_NUMBER " (" __NOW__ ")"
	#define GIT_VERSION ""
#endif

static inline uint32_t mmt_sec_get_version_number_from_string( const char *version){
	const char *str = version;
	uint32_t val = 0;
	int percent = 100*100*100;

	do{
		val += percent * atoi( str );

		//jump over number
		while( 1 ){
			if( *str > '9' || *str < '0' )
				break;
			str ++;
		}
		//jump over .
		str ++;

		percent /= 100;

	}while( *str != '\0' && percent != 1 );

	return val;
}

const char *mmt_sec_get_version_hash(){
	return GIT_VERSION;
}

const char *mmt_sec_get_version_info(){
	return MMT_SEC_VERSION;
}

const char *mmt_sec_get_version_number(){
	return VERSION_NUMBER;
}

uint32_t mmt_sec_get_version_index(){
	return mmt_sec_get_version_number_from_string( VERSION_NUMBER );
}

uint32_t mmt_sec_get_required_plugin_version_number(){
	return mmt_sec_get_version_number_from_string( REQUIRE_PLUGIN );
}
