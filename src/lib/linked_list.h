/*
 *  Created on: 19 april 2017
 *  Created by: Huu Nghia NGUYEN <huunghia.nguyen@montimage.com>
 *
 * Basic data structure: linked-list
 */

#ifndef SRC_LIB_LINKED_LIST_H_
#define SRC_LIB_LINKED_LIST_H_

#include <stdint.h>
#include "base.h"
#include "mmt_alloc.h"

//////////////////////////////////Linked-List ///////////////////////////////////////
typedef struct linked_node_struct{
	struct linked_node_struct *next;
	void *data;
}linked_node_t;

typedef struct linked_list_struct{
	linked_node_t *head;
	size_t elements_count;
}linked_list_t;


/**
 * Create and init a new linked list
 * @return
 */
static inline linked_list_t *linked_list_create(){
	linked_list_t *ret  = mmt_mem_alloc( sizeof( linked_list_t ));
	ret->head           = NULL;
	ret->elements_count = 0;
	return ret;
}

static inline bool linked_list_is_empty( linked_list_t *list ){
	return list->head == NULL;
}

static inline void linked_list_insert( linked_list_t *list, void *data ){
	linked_node_t *node = mmt_mem_alloc( sizeof( linked_node_t ));
	node->data = data;

	//if linked list is empty
	if( linked_list_is_empty( list ) ){
		node->next = NULL;
		list->head = node;
	} else {
		node->next = list->head;
		list->head = node;
	}
	list->elements_count ++;
}

static inline void *linked_list_remove( linked_list_t *list ){
	linked_node_t *node;
	void *data;
	//list is empty
	if( unlikely( linked_list_is_empty( list ) ))
		return NULL;

	node = list->head;

	list->head = node->next;
	list->elements_count --;

	data = node->data;
	mmt_mem_free( node );
	return data;
}

/**
 * Free a list.
 */
static inline void linked_list_free( linked_list_t *list, void ( *free_fn)( void *) ){
	__check_null( list, );
	linked_node_t *node = list->head;
	if( free_fn != NULL )
		for( ; node != NULL; node = node->next ){
			free_fn( node->data );
			mmt_mem_free( node );
		}
	else
		for( ; node != NULL; node=node->next )
			mmt_mem_free( node );

	list->head = NULL;
	mmt_mem_free( list );
}

#endif /* SRC_LIB_LINKED_LIST_H_ */
