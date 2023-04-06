/*
 * mmt_utils.h
 *
 *  Created on: 22 sept. 2016
 *  Created by: Huu Nghia NGUYEN <huunghia.nguyen@montimage.com>
 */

#ifndef SRC_LIB_MMT_UTILS_H_
#define SRC_LIB_MMT_UTILS_H_

#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <ctype.h>
#include "mmt_alloc.h"
#include "mmt_log.h"

#define str_end_with( str, y) (strcmp(str + strlen(str) - strlen( y ), y) == 0)

static inline uint64_t rdtsc() {
    uint32_t lo, hi;
    __asm__ __volatile__ (
      "xorl %%eax, %%eax\n"
      "cpuid\n"
      "rdtsc\n"
      : "=a" (lo), "=d" (hi)
      :
      : "%ebx", "%ecx");
    return (uint64_t)hi << 32 | lo;
}

/**
 * find a byte in an array
 * - Return
 * 	+ 0 if does not exist
 * 	+ otherwise index+1 where index is the position of search in the array
 */
static inline size_t find_byte( uint8_t search, const uint8_t *data, size_t size){
	size_t i;
	for( i=0; i<size; i++ )
		if( data[i] == search )
			return i+1;
	return 0;
}

/**
 * Get current data-time in a string.
 * - Input:
 * 	+ #template is the one of #strftime, for example: "%d %m %Y %H:%M"
 * - Note:
 * 	You need to use #mmt_free to free the returned result.
 */
static inline char* get_current_date_time_string( const char *template ){
	char text[100];
	time_t now = time(NULL);
	struct tm *t = localtime(&now);

	strftime(text, sizeof(text)-1, template, t);
	return mmt_mem_dup( text, strlen( text ));
}


/**
 * Encode 2 uint16_t to 1 uint32_t
 */
static inline uint32_t simple_hash_32( uint16_t a, uint16_t b ){
	uint32_t val = 0;
	val = a << 16;
	val = val | b;
	return val;
}

/**
 * Decode 1 uint32_t to 2 uint16_t
 */
static inline void simple_dehash_32( uint32_t val, uint16_t *a, uint16_t *b){
	*a = val >> 16;
	*b = (val << 16) >> 16;
}


/**
 * Encode 2 uint32_t to 1 uint64_t
 */
static inline uint64_t simple_hash_64( uint32_t a, uint32_t b ){
	uint64_t val = a;
	val = val << 32;
	val = val | b;
	return val;
}

/**
 * Decode 1 uint64_t to 2 uint32_t
 */
static inline void simple_dehash_64( uint64_t val, uint32_t *a, uint32_t *b){
	*a = val >> 32;
	*b = (val << 32) >> 32;
}

/**
 * Split a string to an array
 * @param string
 * @param a_delim
 * @param array
 * @return
 */
static inline size_t str_split(const char* string, char a_delim, char ***array){
	__check_null(string, 0);
	char *a_str = strdup( string );
	size_t count     = 0;
	char* tmp        = a_str;
	char* last_comma = NULL;
	char **result    = NULL;
	char delim[2];
	delim[0] = a_delim;
	delim[1] = 0;

	ASSERT( a_str != NULL, "Not enough memory");

	/* Count how many elements will be extracted. */
	while( *tmp ){
		if (a_delim == *tmp){
			count++;
			last_comma = tmp;
		}
		tmp++;
	}

	/* Add space for trailing token. */
	count += last_comma < (a_str + strlen(a_str) - 1);

	result = mmt_mem_alloc( sizeof( char* ) * count );

	size_t idx  = 0;

	char* token = strtok( a_str, delim );
	while( token ){
		result[ idx++ ] = mmt_mem_dup( token, strlen( token) );
		token = strtok( NULL, delim );
	}

	*array = result;

	free( a_str );
	return count;
}

/**
 * Check if a value is existing in an array
 * @param val
 * @param array
 * @param array_size
 * @return index of element, from 0 to (array_size-1), if the element is existing
 *  otherwise, return array_size;
 */
static inline
size_t index_of( uint32_t val, const uint32_t *array, size_t array_size){
	size_t i;
	for( i=0; i<array_size; i++ )
		if( array[i] == val )
			break;
	return i;
}

/**
 * mask is a string indicating logical cores to be used,
 *  e.g., "1-8,11-12,19" => we use cores 1,2,..,8,11,12,19
 *
 * BNFesque
 * rangelist := (range | number) [',' rangelist]
 * range := number '-' number
 *
 * - Input:
 * 	+ mask is a string ended by '\0'
 * - Output:
 * 	+ an array is created
 * - Return:
 * 	+ size of the output array
 */
static inline size_t expand_number_range( const char *mask, uint32_t **result ){
	const char *cur, *prv;
	size_t size = 0, i, j;
	uint32_t num;
	//TODO this can handle maximally only 100K rules in rules-mask (lcores in core-mask)
	uint32_t array[ 100000 ] = { 0 }; //init all elements to 0. This is not necessary but used to  pass PVS studio check

	*result = NULL;
	if( mask == NULL ) return 0;

	cur = mask;
	while( *cur != '\0' ){
		//first number
		if( !isdigit( *cur ) ){
			log_write( LOG_ERR, "Mask: Expected a digit at %s", cur );
			return 0;
		}

		num = atoi( cur );
		if( index_of( num, array, size ) == size )
			array[ size++ ] = num;

		while( isdigit( *cur ) ) cur ++;


		if( *cur == '\0' ) break;

		//separator
		if( *cur != ',' &&  *cur != '-' ){
			log_write( LOG_ERR, "Mask: Expected a separator, either ' or , at %s", cur );
			return 0;
		}

		//second number
		if( *cur == '-' ){
			cur ++;
			//get another end of range
			num = atoi( cur );
			while( isdigit( *cur ) ) cur ++; //jump over the end of range

			i=array[ size-1 ] + 1; //second number in the range

			if( i > num ){
				log_write( LOG_ERR, "Mask: Range is incorrect %zu-%d", i-1, num );
				return 0;
			}

			for(  ; i<=num; i++ )
				if( index_of( i, array, size ) == size )
					array[ size ++ ] = i;

			//after the second number must be ',' or '\n'
			if( *cur == '\0' )
				break;
		}

		if( *cur != ',' ){
			log_write( LOG_ERR, "Mask: Expected a separator , at %s", cur );
			return 0;
		}
		cur++;

		if( *cur == '\0' ){
			log_write( LOG_ERR, "Mask: Unexpected a separator , at the end" );
			return 0;
		}
	}

	*result = mmt_mem_dup( array, size * sizeof( uint32_t) );
	return size;
}


/**
 * Note: this function create a new memory segment to contain rule_range
 * ==> user must free it using mmt_mem_free after using
 *
 * @param thread_id
 * @param rule_mask "(0:1-4,5,9-10)(2:100-300)"
 * @param rule_range
 * @return number of rules
 */
static inline const size_t get_special_rules_for_thread( uint32_t thread_id, const char *rule_mask, uint32_t **rule_range ){
	uint32_t id = 0;
	size_t size = 0, range_count = 0;
	const char *c = rule_mask, *ptr;
	char *string;
	*rule_range = NULL;
	while( *c != '\0'){
		ASSERT( *c == '(', "Rule mask is not correct: %s", c );
		//jump over (
		c ++;
		//thread id
		ASSERT( isdigit( *c ), "Rule mask is not correct: %s", c );
		id = atol( c );
		//jump over thread id
		while( isdigit( *c ) ) c ++;
		//jump over separator between thread_id and rule_range
		ASSERT( *c == ':', "Rule mask is not correct: %s", c );
		c++;
		//rule range
		ptr  = c;
		size = 0;
		while( *c != ')'){
			switch( *c ){
			case ',':
			case '-':
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				break;
			default:
				ABORT("Rule mask is not correct: %s", c );
			}
			size ++;
			c++;
		}

		//jump over )
		c ++;
		if( id == thread_id ){
			//check if double range for this thread_id
			ASSERT( *rule_range == NULL, "Rule mask is not correct: %s", c );
			string = mmt_mem_dup( ptr, size );
			range_count = expand_number_range( string, rule_range );
			mmt_mem_free( string );
		}
	}
	return range_count;
}



/**
 * Get a list of rules id existing in a rule mask
 * Note: this function create a new memory segment to contain #rules_set
 * ==> user must free it using mmt_mem_free after using
 *
 * @param rule_mask "(0:1-4,5,9-10)(2:100-300)"
 * @param rules_set
 * @return number of rules
 */
static inline const size_t get_rules_id_list_in_mask( const char *rule_mask, uint32_t **rules_set ){
	size_t size = 0, rules_count = 0, i, j;
	const char *c = rule_mask, *ptr;
	char *string;
	//TODO this can handle maximally only 100K rules in rules-mask
	uint32_t array[ 100000 ];

	size_t range_count;
	uint32_t *rule_range;

	while( *c != '\0'){
		if( *c != '(' ){
			log_write( LOG_ERR,"Rule mask is not correct. Expected (, not \"%s\"", c );
			return 0;
		}
		//jump over (
		c ++;
		//thread id
		if( !isdigit( *c )){
			log_write( LOG_ERR,"Rule mask is not correct. Expected a digit, not \"%s\"", c );
			return 0;
		}

		//jump over thread id
		while( isdigit( *c ) ) c ++;
		//jump over separator between thread_id and rule_range
		if( *c != ':'){
			log_write( LOG_ERR,"Rule mask is not correct. Expected :, not \"%s\"", c );
			return 0;
		}
		c++;
		//rule range
		ptr  = c;
		size = 0;
		while( *c != ')'){
			switch( *c ){
			case ',':
			case '-':
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				break;
			default:
				log_write( LOG_ERR,"Rule mask is not correct. Unexpected: %s", c );
				return 0;
			}
			size ++;
			c++;
		}

		//jump over )
		c ++;

		string = mmt_mem_dup( ptr, size );
		range_count = expand_number_range( string, &rule_range );
		//add to array if does not exist
		for( i=0; i<range_count; i++ ){
			for( j=0; j<rules_count; j++ )
				if( array[j] == rule_range[i] )
					break;

			//does not exist
			if( j == rules_count ){
				array[ rules_count ] = rule_range[ i ];
				rules_count ++;
			}
		}

		mmt_mem_free( rule_range );
		mmt_mem_free( string );
	}

	*rules_set = mmt_mem_dup( array, rules_count * sizeof( uint32_t) );

	return rules_count;
}

/**
 * Native sorting an array in ascending order
 * @param number_of_elements
 * @param array
 */
static inline
void asc_sort_array_uint64_t( int number_of_elements, uint64_t *array ){
	uint64_t tmp;
	int i, j;
	for( i=0; i<number_of_elements; i++ )
		for( j=i+1; j<number_of_elements; j++ )
			if( array[i] > array[j] ){
				//swap 2 elements
				tmp = array[i];
				array[i] = array[j];
				array[j] = tmp;
			}
}

/**
 * Binary Search
 * @param number_of_elements is number of elements inside #array
 * @param array is an array that must be sorted in ascending order
 * @param key
 * @return index of element having the same value with #key if found,
 * 		  otherwise, #number_of_elements
 *
 */
static inline
size_t binary_search_uint64_t(size_t number_of_elements, const uint64_t *array, uint64_t key) {
	size_t low = 0, high = number_of_elements-1, mid;
	while(low <= high) {
		mid = (low + high)/2;

		// low path
		__builtin_prefetch (&array[(mid + 1 + high)/2], 0, 1);
		// high path
		__builtin_prefetch (&array[(low + mid - 1)/2], 0, 1);

		if(array[mid] < key)
			low = mid + 1;
		else if(array[mid] == key)
			return mid;
		else// if(array[mid] > key)
			high = mid-1;
	}
	return number_of_elements;
}
#endif /* SRC_LIB_MMT_UTILS_H_ */
