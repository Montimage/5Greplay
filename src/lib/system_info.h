/*
 * system_info.h
 *
 *  Created on: 15 avr. 2016
 *      Author: nhnghia
 */

#ifndef SRC_LIB_SYSTEM_INFO_H_
#define SRC_LIB_SYSTEM_INFO_H_

#include <stdint.h>
#include <sys/types.h>

/**
 * Fix the thread that calls this function on a (logical) processor, then
 * set its priority
 * @param proc_index: index of processor start from 0
 * @priority: is a value in the range -20 to 19. The default priority is 0; lower priorities cause more favorable scheduling.
 * @return: 0     if success
 * 			other if error. Use strerror(errno) to get error description
 */
int move_the_current_thread_to_a_processor( uint16_t proc_index, int priority );

/**
 * Get total number of logical processors
 * return: < 0 if error
 *       : > 0 if success
 *
 */
long get_number_of_processors();

/**
 * Get total number of logical processors that can work
 * return: < 0 if error
 *       : > 0 if success
 */
long get_number_of_online_processors();

/**
 * Get id of the caller process
 */
pid_t gettid();

#endif /* SRC_LIB_SYSTEM_INFO_H_ */
