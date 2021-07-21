/*
 * prefetch.h
 *
 *  Created on: Mar 29, 2017
 *  Created by: Huu Nghia NGUYEN <huunghia.nguyen@me.com>
 *
 * Prefetch operations.
 *
 * This file defines an API for prefetch macros / inline-functions,
 * which are architecture-dependent. Prefetching occurs when a
 * processor requests an instruction or data from memory to cache
 * before it is actually needed, potentially speeding up the execution of the
 * program.
 */

#ifndef SRC_LIB_PREFETCH_H_
#define SRC_LIB_PREFETCH_H_

#ifndef asm
#define asm __asm__
#endif


/**
 * This function is used to minimize cache-miss latency by moving data into
 * a cache before it is accessed.
 * You can insert calls to #prefetch into code for which you know addresses of
 * data in memory that is likely to be accessed soon.
 *
 * If the prefetch is done early enough before the access then the data will be
 * in the cache by the time it is accessed.
 *
 * The value locality must be a compile-time constant integer between zero and three.
 * A value of zero means that the data has no temporal locality,
 * so it need not be left in the cache after the access.
 * A value of three means that the data has a high degree of temporal locality and
 * should be left in all levels of cache possible.
 * Values of one and two mean, respectively, a low or moderate degree of temporal locality.
 * The default is three.
 */
#define prefetch_r( pointer, locality ) __builtin_prefetch( pointer, 0, locality )
#define prefetch_w( pointer, locality ) __builtin_prefetch( pointer, 1, locality )

/**
 * Prefetch a cache line into all cache levels.
 * @param p
 *   Address to prefetch
 */
static inline void prefetch0(const volatile void *p) {
	asm volatile ("prefetcht0 %[p]" : : [p] "m" (*(const volatile char *)p));
}

/**
 * Prefetch a cache line into all cache levels except the 0th cache level.
 * @param p
 *   Address to prefetch
 */
static inline void prefetch1(const volatile void *p) {
	asm volatile ("prefetcht1 %[p]" : : [p] "m" (*(const volatile char *)p));
}

/**
 * Prefetch a cache line into all cache levels except the 0th and 1th cache
 * levels.
 * @param p
 *   Address to prefetch
 */
static inline void prefetch2(const volatile void *p) {
	asm volatile ("prefetcht2 %[p]" : : [p] "m" (*(const volatile char *)p));
}

/**
 * Prefetch a cache line into all cache levels (non-temporal/transient version)
 *
 * The non-temporal prefetch is intended as a prefetch hint that processor will
 * use the prefetched data only once or short period, unlike the
 * rte_prefetch0() function which imply that prefetched data to use repeatedly.
 *
 * @param p
 *   Address to prefetch
 */
static inline void prefetch_non_temporal(const volatile void *p) {
	asm volatile ("prefetchnta %[p]" : : [p] "m" (*(const volatile char *)p));
}


#endif /* SRC_LIB_PREFETCH_H_ */
