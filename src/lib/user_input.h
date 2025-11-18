/*
 * user_input.h
 *
 *  Created on: Nov 18, 2025
 *      Author: nhnghia
 */

#ifndef SRC_LIB_USER_INPUT_H_
#define SRC_LIB_USER_INPUT_H_

/**
 *
 * @param env_name
 * @param env_name_size
 * @param input_name
 * @return
 */
int user_input_set(const char *name, const char *value );

const char * user_input_get(const char *name);

#endif /* SRC_LIB_USER_INPUT_H_ */
