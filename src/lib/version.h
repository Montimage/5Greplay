/*
 * version.h
 *
 *  Created on: Nov 3, 2016
 *  Created by: Huu Nghia NGUYEN <huunghia.nguyen@montimage.com>
 *
 *  Define version of mmt-engine
 *  One may change #VERSION_NUMBER and #GIT_VERSION from Makefile
 */

#ifndef SRC_LIB_VERSION_H_
#define SRC_LIB_VERSION_H_

#include <inttypes.h>

/**
 * Get version number of MMT-5Greplay
 * @return a string, e.g., "1.1.0"
 */
const char* mmt_sec_get_version_number();

/**
 * Get version number of MMT-5Greplay
 * @return a string representing a hash of its git version
 * e.g., c186c33
 */
const char* mmt_sec_get_version_hash();

/**
 * Get version number of MMT-5Greplay
 * @return a number, e.g., 101000
 */
uint32_t mmt_sec_get_version_index();

/**
 * Get version number of rules
 * @return
 */
uint32_t mmt_sec_get_required_plugin_version_number();

const char *mmt_sec_get_version_info();

#endif /* SRC_LIB_VERSION_H_ */
