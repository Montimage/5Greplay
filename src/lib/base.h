/*
 * base.h
 *
 *  Created on: 21 sept. 2016
 *  Created by: Huu Nghia NGUYEN <huunghia.nguyen@montimage.com>
 *
 *  Basic definitions/data struct
 */

#ifndef SRC_LIB_BASE_H_
#define SRC_LIB_BASE_H_

#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h> //for uint64_t PRIu64
#include <stdbool.h>

//TODO: this is used to test
//#ifndef MODULE_ADD_OR_RM_RULES_RUNTIME
//#define MODULE_ADD_OR_RM_RULES_RUNTIME
//#endif

#define UNKNOWN -1
#define NO false
#define YES true

//thread local storage
#define __thread_scope __thread
//a dummy identification to indicate a function being thread safe
#define __thread_safe

//Force alignment to cache line.
#ifdef LEVEL1_DCACHE_LINESIZE
	#if LEVEL1_DCACHE_LINESIZE == 0
	#define __aligned
	#else
	#define __aligned __attribute__ ((aligned(LEVEL1_DCACHE_LINESIZE)))
	#endif
#else
	#define __aligned __attribute__ ((aligned(64)))
#endif

//branch prediction
#ifndef likely
	#define likely(x)   __builtin_expect(!!(x),1)
#endif
#ifndef unlikely
	#define unlikely(x) __builtin_expect(!!(x),0)
#endif

//macro
/**
 * Return y if x is NULL
 */
#define __check_null( x, y ) while( unlikely( x == NULL )) return y
#define __check_zero( x, y ) while( unlikely( x == 0 )) return y
#define __check_bool( x, y ) while( unlikely( x )) return y


/* a=target variable, i=bit number to act upon 0-n  (n == sizeof(a))*/
//set bit i-th to 1
#define BIT_SET(number,i)   ((number) |= (1ULL<<(i)))
//set bit i-th to 0
#define BIT_CLEAR(number,i) ((number) &= ~(1ULL<<(i)))
//flip bit i-th
#define BIT_FLIP(number,i)  ((number) ^= (1ULL<<(i)))
//check bit i-th
#define BIT_CHECK(number,i) ((number) &  (1ULL<<(i)))


/**
 * Allow adding/removing rules at runtime
 * Remove this definition to prevent adding/removing rules at runtime
 *
 * Use lock when we need add/remove rules in runtime
 */
#ifdef MODULE_ADD_OR_RM_RULES_RUNTIME
	//#pragma message("Enable: add/remove rules at runtime")
	#define BEGIN_LOCK_IF_ADD_OR_RM_RULES_RUNTIME( spin_lock ) if( pthread_spin_lock( spin_lock ) == 0 ){
	#define UNLOCK_IF_ADD_OR_RM_RULES_RUNTIME( spinlock ) while( pthread_spin_unlock( spinlock ) != 0 ) nanosleep( (const struct timespec[]){{0, 10000}}, NULL );
	#define END_LOCK_IF_ADD_OR_RM_RULES_RUNTIME }
#else
	//#warning "Disable: add/remove rules at runtime"
	#define BEGIN_LOCK_IF_ADD_OR_RM_RULES_RUNTIME( x )
	#define UNLOCK_IF_ADD_OR_RM_RULES_RUNTIME( x )
	#define END_LOCK_IF_ADD_OR_RM_RULES_RUNTIME
#endif

#define IS_EQUAL_STRINGS(a, b) (strcmp(a, b) == 0)

#endif /* SRC_LIB_BASE_H_ */
