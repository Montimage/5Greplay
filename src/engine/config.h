/*
 * mmt_sec_config.h
 *
 *  Created on: 23 nov. 2016
 *      Author: la_vinh
 */

#ifndef MMT_SEC_CONFIG_H_
#define MMT_SEC_CONFIG_H_
#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>

enum config_att{
	/**
	 * maximum size, in bytes, of a report received from mmt-probe
	 */
	MMT_SEC__CONFIG__INPUT__MAX_MESSAGE_SIZE,
	/**
	 * number of fsm instances of one rule
	 */
	MMT_SEC__CONFIG__SECURITY__MAX_INSTANCES,

	/**
	 * Multi thread: size of buffer
	 */
	MMT_SEC__CONFIG__SECURITY__SMP__RING_SIZE,

	/**
	 * Number of consecutive alerts of one rule can be ignored its description.
	 * The first alert of a rule always contain description of its rule.
	 * However, to avoid a huge output, a number of consecutive alerts of that rule
	 * can be excluded the description.
	 *
	 * - set = 0 to include description to any alerts
	 */
	MMT_SEC__CONFIG__OUTPUT__INORGE_DESCRIPTION,

	/**
	 * Memory pool
	 * When mmt-sec frees a block of memory, it will not call #free immediately.
	 * It will be stored by a memory pool (if the pool is not full).
	 * When mmt-sec requires a block of memory, it will call #malloc only if #mem_pool
	 * has no block having the same size.
	 *
	 * Maximum bytes a thread may be reserved by using mmt_mem_pool
	 */
	MMT_SEC__CONFIG__MEMPOOL__MAX_BYTES,
	/**
	 * maximum elements of a pool of a mem_pool
	 * A mem_pool contains several pools. Each pool stores several blocks of memory
	 * having the same size.
	 * This parameter set the maximum elements of a pool.
	 */
	MMT_SEC__CONFIG__MEMPOOL__MAX_ELEMENTS_PER_POOL,
};

bool mmt_sec_load_default_config();

uint32_t mmt_sec_set_config( enum config_att att, uint32_t value );

uint32_t mmt_sec_get_config( enum config_att att );

const char* mmt_sec_get_config_name( enum config_att att );

#endif /* MMT_SEC_CONFIG_H_ */
