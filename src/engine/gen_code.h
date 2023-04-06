/*
 * gen_code.h
 *
 *  Created on: 4 oct. 2016
 *  Created by: Huu Nghia NGUYEN <huunghia.nguyen@montimage.com>
 */

#ifndef SRC_LIB_GEN_CODE_H_
#define SRC_LIB_GEN_CODE_H_

#include "../lib/base.h"
#include "rule.h"

int generate_fsm( const char* file_name, rule_t *const*rules, size_t count, const char *embedded_functions );

int compile_gen_code( const char *lib_file, const char *code_file, const char *incl_dir );

#endif /* SRC_LIB_GEN_CODE_H_ */
