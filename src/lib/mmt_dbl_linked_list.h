/*
 * data_struct.h
 *
 *  Created on: 20 sept. 2016
 *  Created by: Huu Nghia NGUYEN <huunghia.nguyen@montimage.com>
 *
 * Basic data structure: linked-list, binary_stree
 */

#ifndef SRC_LIB_MMT_DBL_LINKED_LIST_H_
#define SRC_LIB_MMT_DBL_LINKED_LIST_H_

#include <stdint.h>
#include "base.h"
#include "mmt_lib.h"

//////////////////////////////////Linked-List ///////////////////////////////////////
typedef struct link_node_struct{
	/** two linkers link to the previous and the next nodes*/
	struct link_node_struct *prev, *next;
	/** data of the node */
	void *data;
}link_node_t;

/**
 * Create a new dbl-link-node
 * @param data
 * @return
 */
static inline
link_node_t *create_node_of_link_list( void *data ){
	link_node_t *new_node = mmt_mem_alloc( sizeof( link_node_t ));
	new_node->data = data;
	new_node->prev = new_node->next = NULL;
	return new_node;
}

/**
 * Create a new node of a linked-list
 * - Input:
 * 	+ *data: a pointer points to data of the node being created
 * - Output:
 * - Return:
 * 	+ a pointer points to the new node
 */
static inline
link_node_t *append_node_to_link_list( link_node_t *head, void *data ){
	link_node_t *new_node, *ptr;

	new_node = create_node_of_link_list( data );

	if( unlikely( head == NULL )) return new_node;

	//append to tail
	ptr = head;
	//find tail
	while( ptr->next != NULL ) ptr = ptr->next;
	//add new node to tail
	ptr->next = new_node;
	new_node->prev = ptr;

	return head;
}

/**
 * Create then append a new node to the end of a linked-list
 * - Input:
 * 	+ entry: head of the linked-list. This can be NULL
 * 	+ data : data to be add to the new node
 * - Output:
 * - Return:
 * 	+ new head of the linked-list.
 * 		If entry is NULL then the function will return the new node being created
 */
static inline
link_node_t *insert_node_to_link_list( link_node_t *head, void *data ){
	link_node_t *new_node;

	new_node = create_node_of_link_list( data );

	if( unlikely( head == NULL )) return new_node;

	//insert to head
	new_node->next = head;
	head->prev     = new_node;

	return new_node;
}

static inline
link_node_t *remove_link_node_from_its_link_list( const link_node_t *node, link_node_t *head ){
	if( unlikely( node == NULL ))
		return head;
	//head?
	if( node == head ){
		if( node->next != NULL )
			node->next->prev = NULL;

		return node->next;
	}
	//node is not null && node->pre is not null as node != head
	node->prev->next = node->next;

	if( node->next != NULL )
		node->next->prev = node->prev;

	return head;
}

/**
 * Remove a node having #data from the list.
 * If there is no node has #data, the function does not change the list.
 */
static inline
link_node_t *remove_node_from_link_list( link_node_t *head, const void *data ){
	link_node_t *ptr = head;
	while( ptr != NULL && ptr->data != data )
		ptr = ptr->next;

	//not found any node having this #data
	if( ptr == NULL )
		return head;

	head = remove_link_node_from_its_link_list( ptr, head );

	//free this node
	mmt_mem_force_free( ptr );

	return head;
}

/**
 * Free a list and its data.
 * Data of each node is freed by function #free_fn.
 * If #free_fn is NULL, the data will not be freed.
 */
static inline
void free_link_list_and_data( link_node_t *head, void (*free_fn)( void *) ){
	link_node_t *ptr;
	if( free_fn )
		while( head != NULL ){
			free_fn( head->data );

			ptr = head->next;
			head->next = head->prev = NULL;
			mmt_mem_force_free( head );

			head = ptr;
		}
	else{
		while( head != NULL ){
			ptr = head->next;
			head->next = head->prev = NULL;
			mmt_mem_force_free( head );

			head = ptr;
		}
	}
}

/**
 * Free a list.
 * Data of each node is freed if #free_data == YES
 */
static inline
void free_link_list( link_node_t *head, bool free_data ){
	if( free_data )
		free_link_list_and_data( head, (void *)mmt_mem_free );
	else
		free_link_list_and_data( head, NULL );
}

static inline
size_t count_nodes_from_link_list( const link_node_t *entry ){
	size_t size = 0;
	while( entry != NULL ){
		size ++;
		entry = entry->next;
	}
	return size;
}


#endif /* SRC_LIB_MMT_DBL_LINKED_LIST_H_ */
