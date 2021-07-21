/*
 * system_info.c
 *
 *  Created on: 15 avr. 2016
 *      Author: nhnghia
 */

#ifndef __USE_GNU
#define __USE_GNU
#endif
#define _GNU_SOURCE
#include <sched.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "system_info.h"


#ifdef RHEL3 //RedHat Enterprise Linux 3
#define my_sched_setaffinity(a,b,c) sched_setaffinity(a, c)
#else
#define my_sched_setaffinity(a,b,c) sched_setaffinity(a, b, c)
#endif /* RHEL3 */

int move_the_current_thread_to_a_processor( uint16_t core_index, int niceval ){
	int ret;
	int rtid = gettid(); /* reader thread id */
	cpu_set_t csmask;

	// initialize to empty set
	CPU_ZERO(&csmask);

	CPU_SET(core_index, &csmask);

	//affinity
	ret = my_sched_setaffinity(rtid, sizeof (cpu_set_t), &csmask);
	if (ret != 0) return ret;

	//set
	ret = setpriority(PRIO_PROCESS, rtid, niceval);
	return ret;
}

long get_number_of_processors(){
	return sysconf(_SC_NPROCESSORS_CONF);
}

/**
 * Get total number of logical processors that can work
 */
long get_number_of_online_processors(){
	//cached value
	/*
	static long ret = -1;
	if( ret >= 0 ) return ret;
	ret = sysconf(_SC_NPROCESSORS_ONLN);
	return ret;
	*/

	return sysconf(_SC_NPROCESSORS_ONLN);
}


pid_t gettid() {
	return syscall( __NR_gettid );
}
