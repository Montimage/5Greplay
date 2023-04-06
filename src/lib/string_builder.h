/*
 * string_builder.h
 *
 *  Created on: Jun 1, 2018
 *          by: Huu Nghia Nguyen
 */

#ifndef SRC_LIB_STRING_BUILDER_H_
#define SRC_LIB_STRING_BUILDER_H_

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>
#include "optimization.h"
#include "macro_apply.h"
#include "log.h"

#define INET_ADDRSTRLEN 16

/**
 * Append a  character to a string
 * @param dst
 * @param dst_size
 * @param c
 * @return
 * @note this function does not append '\0' to the final result
 */
static ALWAYS_INLINE int append_char( char *dst, size_t dst_size, char c ){
	if( unlikely( dst_size == 0 ))
		return 0;
	dst[0] = c;
	return 1;
}

/**
 * Append an array of characters to a string
 * @param dst
 * @param dst_size
 * @param src
 * @return
 * @note this function may not append '\0' to the final result
 */
static ALWAYS_INLINE int append_string_without_quote( char *dst, size_t dst_size, const char *src ){
	if( unlikely( dst_size == 0 ))
		return 0;

	size_t src_size = strlen( src );
	//cannot contain all source string
	if( src_size > dst_size )
		src_size = dst_size;

	//open quote
	memcpy( dst, src, src_size );
	//close quote
	return src_size;
}

/**
 * Append a string src to another string. The final result will contain the string src surrounded by quotes
 * @param dst
 * @param dst_size
 * @param src
 * @return
 * @note this function may not append '\0' to the final result
 */
static ALWAYS_INLINE int append_string( char *dst, size_t dst_size, const char *src ){
	if( unlikely( dst_size < 2 ))
		return 0;
	else if( unlikely( dst_size == 2 )){
		dst[0] = '"';
		dst[1] = '"';
		return 2;
	}

	size_t src_size = strlen( src );
	dst_size -= 2; //2 characters for " and "
	//cannot contain all source string
	if( src_size > dst_size )
		src_size = dst_size;

	//open quote
	dst[0] = '"';
	if( src_size != 0 )
		memcpy( &dst[1], src, src_size );
	//close quote
	dst[ src_size + 1 ] = '"';
	return src_size + 2;
}

/**
 * Append a hex number to a string. The hex number must be less than 0xFF
 * @param dst
 * @param dst_size
 * @param val
 * @return
 * @note this function does not append '\0' to the final result
 */
static ALWAYS_INLINE int append_hex( char *dst, size_t dst_size, uint8_t val ){
	//wee need at least 2 characters: XY
	if( unlikely( dst_size < 2 ))
		return 0;
	const char *digits = "0123456789ABCDEF";
	dst[0] = digits[ val >> 4  ];
	dst[1] = digits[ val & 0xF ];
	return 2;
}

/**
 * Append a MAC address to a string. The result MAC address will be surrounded by quotes
 * @param dst
 * @param dst_size
 * @param t
 * @return
 * @note this function does not append '\0' to the final result
 */
static ALWAYS_INLINE int append_mac( char *dst, size_t dst_size, const uint8_t *t ){
	//wee need at least 2+6*2+5 characters: "11:22:33:44:55:66"
	if( unlikely( dst_size < 19 ))
		return 0;

	int offset = 0;
	dst[ offset ++ ] = '"';
	offset += append_hex( dst + offset, 2, t[0] );
	dst[ offset ++ ] = ':';

	offset += append_hex( dst + offset, 2, t[1] );
	dst[ offset ++ ] = ':';

	offset += append_hex( dst + offset, 2, t[2] );
	dst[ offset ++ ] = ':';

	offset += append_hex( dst + offset, 2, t[3] );
	dst[ offset ++ ] = ':';

	offset += append_hex( dst + offset, 2, t[4] );
	dst[ offset ++ ] = ':';

	offset += append_hex( dst + offset, 2, t[5] );
	dst[ offset ++ ] = '"';
	return offset;
}

/**
 * Convert number to string
 * @param string
 * @param val
 * @return
 */
static inline int append_number( char *dst, size_t dst_size, uint64_t val ){
	if( val < 10 && dst_size > 0 ) {
		dst[0] = '0' + val;
		return 1;
	}

	const char digit_pairs[201] = {
			"00010203040506070809"
			"10111213141516171819"
			"20212223242526272829"
			"30313233343536373839"
			"40414243444546474849"
			"50515253545556575859"
			"60616263646566676869"
			"70717273747576777879"
			"80818283848586878889"
			"90919293949596979899"
	};

#define M2  10
#define M3  100
#define M4  1000
#define M5  10000
#define M6  100000
#define M7  1000000
#define M8  10000000
#define M9  100000000
#define M10 1000000000
#define M11 10000000000
#define M12 100000000000
#define M13 1000000000000
#define M14 10000000000000
#define M15 100000000000000
#define M16 1000000000000000
#define M17 10000000000000000
#define M18 100000000000000000
#define M19 1000000000000000000
#define M20 10000000000000000000U
//   2^64 = 18446744073709551616

	int size = 0;
	//get number of digits
	if( val >= M4 ){
		if( val >= M6 ){
			if( val >= M8 ){
				if( val >= M10 ){
					if( val >= M12 ){
						if( val >= M14 ){
							if( val >= M16 ){
								if( val >= M18 ){
									if( val >= M20 )
										size = 20;
									else if( val >= M19 )
										size = 19;
									else
										size = 18;
								}else{ //val < M18
									if( val >= M17 )
										size = 17;
									else
										size = 16;
								}
							}else{ //val < M16
								if( val >= M15 )
									size = 15;
								else
									size = 14;
							}
						}else{ //val < M14
							if( val >= M13 )
								size = 13;
							else
								size = 12;
						}

					}else{ //val < M12
						if( val >= M11 )
							size = 11;
						else
							size = 10;
					}
				}else{ //val < M10
					if( val >= M9 )
						size = 9;
					else
						size = 8;
				}
			}else{ //val < M8
				if( val >= M7 )
					size = 7;
				else
					size = 6;
			}
		}else{ //val < M6
			if( val >= M5 )
				size = 5;
			else
				size = 4;
		}
	}else{ //val < M4
		if( val >= M3 ){
			size = 3;
		}else{ //val < M3
			size = 2;
		}
	}

	if( unlikely( size > dst_size ))
		return 0;

	char *c = &dst[ size-1 ];
	int pos;

	//do each 2 digits
	while( val >= 100 ){
		pos = val % 100;
		val /= 100;
		*(uint16_t*)(c-1) = *(uint16_t*)( digit_pairs + 2*pos);
		c -= 2;
	}

	while( val > 0 ){
		*c--='0' + (val % 10);
		val /= 10;
	}

	return size;
}

/**
 * Convert IPv4 from 32bit number to human readable string
 * @param ip
 * @param dst must point to a memory segment having at least INET_ADDRSTRLEN bytes
 * @return length of buf
 * @return
 */
static ALWAYS_INLINE int append_ipv4( char *dst, size_t dst_size, uint32_t ip  ){
	if( dst_size < INET_ADDRSTRLEN )
		return 0;
	const uint8_t *p = (const uint8_t *) &ip;
	int valid = 0;
	valid += append_number(dst+valid, dst_size-valid, p[0]);
	dst[valid++] = '.';
	valid += append_number(dst+valid, dst_size-valid, p[1]);
	dst[valid++] = '.';
	valid += append_number(dst+valid, dst_size-valid, p[2]);
	dst[valid++] = '.';
	valid += append_number(dst+valid, dst_size-valid, p[3]);
	return valid;
}

/**
 * Append a struct timeval to string using format tv_sec.tv_usec
 * This function corresponds to sprintf( dst, dst_size, "%u.%06u", t->tv_sec, tv_usec)
 * @param dst
 * @param dst_size
 * @param t
 * @return
 */
static ALWAYS_INLINE int append_timeval( char *dst, size_t dst_size, const struct timeval *t ){
	//wee need at least 12 characters: xxxxxx.
	if( unlikely( dst_size < 12 ))
		return 0;
	int offset = append_number( dst, dst_size, t->tv_sec );
	ASSERT( offset <= 12, "Impossible (%d > 12)", offset );

	//not enough for the nanosecond
	if( dst_size < offset + 7 )
		return offset;

	dst += offset;
	*dst = '.';
	dst ++;

	//tmp char
	char tmp[6];
	int len = append_number( tmp, sizeof( tmp ), t->tv_usec );
	//put tmp to end of 6 bytes of dst (left align), for example: with offset = 4
	// tmp =   1234
	// dst = 001234
	//1. pre-fill zero
	dst[0] = '0';
	dst[1] = '0';
	dst[2] = '0';
	dst[3] = '0';
	dst[4] = '0';
	dst[5] = '0';
	//2. copy tmp to end of dst
	int i;
	for( i=0; i<len; i ++ )
		dst[i + (sizeof(tmp)-len) ] = tmp[i];

	return offset + 1 + sizeof(tmp); //1 is '.'
}

/**
 * These helpers are used only inside STRING_BUILDER macro
 */
#define __ARR(x)   append_string_without_quote ( __ptr+__i, __n-__i, x )
#define __STR(x)   append_string(                __ptr+__i, __n-__i, x )
#define __INT(x)   append_number(                __ptr+__i, __n-__i, x )
#define __CHAR(x)  append_char  (                __ptr+__i, __n-__i, x )
#define __TIME(x)  append_timeval(               __ptr+__i, __n-__i, x )
#define __HEX(x)   append_hex(                   __ptr+__i, __n-__i, x )
#define __MAC(x)   append_mac(                   __ptr+__i, __n-__i, x )
#define __IPv4(x)  append_ipv4(                  __ptr+__i, __n-__i, x )

#define __BUILDER( X ) if( __n > __i ) __i += X;
#define __EMPTY()
#define __SEPARATOR()  __i += append_string_without_quote( __ptr+__i, __n-__i, sepa );

/**
 * Create a macro to build a string.
 * For example, to build a string:  "1,\"GET\", we can do:
 * char msg[100];
 * int valid = 0;
 * valid += append_number( msg+valid, sizeof(msg)-valid, 1);
 * valid += append_char  ( msg+valid, sizeof(msg)-valid, ',');
 * valid += append_string( msg+valid, sizeof(msg)-valid, "GET");
 *
 * The code above can be replaced by using this macro:
 *  int valid = 0;
 *  STRING_BUILDER( valid, msg, sizeof(msg), __INT(1), __CHAR(','), __STR("GET"));
 *
 * @param valid: position in dst from which we want to append data
 * @param dst  : data buffer to append data to
 * @param dst_size: total size of dst
 */
#define STRING_BUILDER( valid, dst, dst_size, ... )      \
do{                                                      \
	int __i = valid, __n=dst_size-1;                     \
	char *__ptr = dst;                                   \
	APPLY( __EMPTY, __BUILDER, __VA_ARGS__ )             \
	__ptr[__i] = '\0';                                   \
	valid = __i;                                         \
}while( 0 )


/**
 * Same as STRING_BUILDER but adding a separator between two consecutive elements
 */
#define STRING_BUILDER_WITH_SEPARATOR( valid, dst, dst_size, separator,... ) \
do{                                                                          \
	int __i = valid, __n=dst_size-1;                                         \
	char *__ptr = dst;                                                       \
	const char *sepa = separator;                                            \
	APPLY( __SEPARATOR, __BUILDER, __VA_ARGS__  )                            \
	__ptr[__i] = '\0';                                                       \
	valid = __i;                                                             \
}while( 0 )

#endif /* SRC_LIB_STRING_BUILDER_H_ */
