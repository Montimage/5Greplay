/*
 * mmt_map_t.c
 *
 *  Created on: Nov 2, 2016
 *  Created by: Huu Nghia NGUYEN <huunghia.nguyen@montimage.com>
 */


#include <string.h>
#include "base.h"
#include "mmt_map_t.h"
#include "mmt_alloc.h"
#include "mmt_log.h"

/**
 * Implement a map by a binary-tree
 */
typedef struct mmt_map_node_struct{
	struct mmt_map_node_struct *left, *right;
	void *key, *data;
}_mmt_map_node_t;

typedef struct mmt_map_struct{
	_mmt_map_node_t *root;
	int (*compare_function)(const void*, const void*);
	//number of nodes in the map
	uint64_t size;
}_mmt_map_t;

/**
 * Public API
 */
size_t mmt_map_count( const mmt_map_t *map ){
	if( map == NULL ) return 0;
	return ((_mmt_map_t*) map)->size;
}

/**
 * Public API
 */
mmt_map_t *mmt_map_init( int (*fun)(const void*, const void*) ){
	_mmt_map_t *map = mmt_mem_alloc( sizeof( _mmt_map_t ));
	map->compare_function = fun;
	map->root = NULL;
	map->size = 0;
	return (mmt_map_t)map;
}

void _mmt_map_free_node( _mmt_map_node_t *node, bool free_data ){
	if( node == NULL ) return;
	//free its key-data if need
	if( free_data == YES ){
		mmt_free_and_assign_to_null( node->key );
		mmt_free_and_assign_to_null( node->data );
	}
	//free its children
	if( node->left != NULL )
		_mmt_map_free_node( node->left, free_data );
	if( node->right != NULL )
		_mmt_map_free_node( node->right, free_data );

	node->left = node->right = NULL;

	//free the node itself
	mmt_mem_free( node );
}

/**
 * Public API
 */
void mmt_map_free( mmt_map_t *map, bool free_data  ){
	__check_null( map, );
	_mmt_map_t *_tree = (_mmt_map_t*) map;
	_mmt_map_free_node( _tree->root, free_data );
	mmt_mem_free( map );
}

void _mmt_map_free_key_and_data( _mmt_map_node_t *node, void (*free_key_fn)( void *), void (*free_data_fn)( void *) ){
	if( node == NULL ) return;
	//free its key-data if need
	if( free_key_fn != NULL )
		free_key_fn( node->key );
	if( free_data_fn != NULL )
		free_data_fn( node->data );

	//free its children
	if( node->left != NULL )
		_mmt_map_free_key_and_data( node->left, free_key_fn, free_data_fn );
	if( node->right != NULL )
		_mmt_map_free_key_and_data( node->right, free_key_fn, free_data_fn );

	node->left = node->right = NULL;

	//free the node itself
	mmt_mem_free( node );
}

void mmt_map_free_key_and_data( mmt_map_t *map, void (*free_key_fn)( void *), void (*free_data_fn)( void *)  ){
	if( map == NULL ) return;
	_mmt_map_t *_tree = (_mmt_map_t*) map;
	_mmt_map_free_key_and_data( _tree->root, free_key_fn, free_data_fn );
	mmt_mem_free( map );
}

void* _mmt_map_set_data( int (*fun)(const void*, const void*), _mmt_map_node_t **node, void *key, void *data, bool override_if_exist ){
	enum compare_result ret = 0;
	void *ptr = NULL;
	_mmt_map_node_t *node_ptr = *node;

	if( node_ptr == NULL ){
		node_ptr = mmt_mem_alloc( sizeof( _mmt_map_node_t ));
		node_ptr->left = node_ptr->right = NULL;
		node_ptr->key  = key;
		node_ptr->data = data;
		*node = node_ptr;
		return NULL;
	}

	ret = (*fun)( key, node_ptr->key );
	//this node has the same key
	if( ret == 0 ){
		if( override_if_exist ){
			ptr = node_ptr->data;
			node_ptr->data = data;
			return ptr;
		} else
			return data;
	}else if( ret < 0 )
		return _mmt_map_set_data( fun, &(node_ptr->left), key, data, override_if_exist );
	else
		return _mmt_map_set_data( fun, &(node_ptr->right), key, data, override_if_exist );
}

/**
 * Public API
 */
void* mmt_map_set_data( mmt_map_t *map, void *key, void *data, bool override_if_exist ){
	void *ptr;
	if( map == NULL || key == NULL ) return NULL;
	_mmt_map_t *_tree = (_mmt_map_t*) map;
	ptr = _mmt_map_set_data( _tree->compare_function, &(_tree->root), key, data, override_if_exist );
	//successfully inserted ==> increase number of nodes
	if( ptr == NULL ) _tree->size ++;
	return ptr;
}


void *_mmt_map_get_data( int (*fun)(const void*, const void*), _mmt_map_node_t *node, const void *key ){
	if( node == NULL ) return NULL;
	enum compare_result ret = (*fun)( key, node->key );
	//this node has the same key
	if( ret == 0 )
		return node->data;
	else if( ret < 0 ){
		if( node->left != NULL )
			return _mmt_map_get_data( fun, node->left, key );
	}else{
		if( node->right != NULL )
			return _mmt_map_get_data( fun, node->right, key );
	}
	return NULL;
}

/**
 * Public API
 */
void *mmt_map_get_data( const mmt_map_t *map, const void *key ){
	__check_null( map, NULL );
	__check_null( key, NULL );

	_mmt_map_t *_tree = (_mmt_map_t*) map;

	__check_null( _tree->root, NULL );

	return _mmt_map_get_data( _tree->compare_function, _tree->root, key );
}

struct tmp_struct{
	int index;
	int (*fun)(const void*, const void*);
	const void *key;
};

static inline void _iterate_to_compare( void *key, void *data, void *user_data, size_t index, size_t total ){
	struct tmp_struct *_user_data = (struct tmp_struct *) user_data;
	if( _user_data->fun( key, _user_data->key ) == 0 )
		_user_data->index = index;
}

int mmt_map_get_index( const mmt_map_t *map, const void *key ){
	__check_null( map, -1 );
	__check_null( key, -1 );

	_mmt_map_t *_tree = (_mmt_map_t*) map;

	__check_null( _tree->root, -1 );

	struct tmp_struct data;

	data.index = -1;
	data.fun   = _tree->compare_function;
	data.key   = key;
	//use _tree->size to store index of founding key
	mmt_map_iterate( map, _iterate_to_compare, &data );

	return data.index;
}


void _mmt_map_node_iterate( const _mmt_map_node_t *node, void (*map_iterate_function)( void *_key, void *_data, void *_user_data, size_t _index, size_t _total ), void *user_data, size_t *index, size_t total ){
	if( node->left != NULL )
		_mmt_map_node_iterate( node->left, map_iterate_function, user_data, index, total );
	(*map_iterate_function)( node->key, node->data, user_data, *index, total );
	//is not the first running of map_iterate_function
	(*index) ++;
	if( node->right != NULL )
		_mmt_map_node_iterate( node->right, map_iterate_function, user_data, index, total );
}
/**
 * Public API
 */
void mmt_map_iterate( const mmt_map_t *map, void (*map_iterate_function)( void *key, void *data, void *user_data, size_t index, size_t total ), void *user_data ){
	size_t index = 0;
	__check_null( map,  );

	_mmt_map_t *_tree = (_mmt_map_t*) map;

	__check_null( _tree->root, );

	_mmt_map_node_iterate( _tree->root, map_iterate_function, user_data, &index, _tree->size );
}

void _iterate_to_clone_map( void *key, void *data, void *user_data, size_t index, size_t total){
	mmt_map_set_data( (mmt_map_t *) user_data, key, data, NO );
}

/**
 * Public API
 */
mmt_map_t* mmt_map_clone( const mmt_map_t *map ){
	__check_null( map, NULL );

	_mmt_map_t *_tree = (_mmt_map_t*) map;
	mmt_map_t *new_map;

	new_map = mmt_map_init( _tree->compare_function );
	//clone map
	mmt_map_iterate( map, _iterate_to_clone_map, new_map );

	return new_map;
}

void _mmt_map_clone_node_and_data( _mmt_map_node_t **new_node, _mmt_map_node_t *node, void* (*clone_key_fn)( void *), void* (*clone_data_fn)( void *) ){
	_mmt_map_node_t *node_ptr;

	__check_null( node, );

	//clone current node
	node_ptr = mmt_mem_alloc( sizeof( _mmt_map_node_t ));
	node_ptr->left = node_ptr->right = NULL;
	if( clone_key_fn )
		node_ptr->key  = clone_key_fn( node->key );
	else
		node_ptr->key  = node->key;
	if( clone_data_fn )
		node_ptr->data = clone_data_fn( node->data );
	else
		node_ptr->data = node->data;
	*new_node = node_ptr;

	//clone left branch
	if( node->left != NULL )
		_mmt_map_clone_node_and_data( &(node_ptr->left), node->left, clone_key_fn, clone_data_fn );

	//clone right branch
	if( node->right != NULL )
		_mmt_map_clone_node_and_data( &(node_ptr->right), node->right, clone_key_fn, clone_data_fn );
}

/**
 * Public API
 */
mmt_map_t* mmt_map_clone_key_and_data( const mmt_map_t *map, void* (*clone_key_fn)( void *), void* (*clone_data_fn)( void *)  ){
	__check_null( map, NULL );

	_mmt_map_t *_tree = (_mmt_map_t*) map;
	mmt_map_t *new_map;

	new_map = mmt_map_init( _tree->compare_function );
	//clone map
	_mmt_map_clone_node_and_data( &(((_mmt_map_t *)new_map)->root), _tree->root, clone_key_fn, clone_data_fn );

	((_mmt_map_t *)new_map)->size = mmt_map_count( map );
	return new_map;
}

static inline void _iterate_to_update_array( void *key, void *data, void *user_data, size_t index, size_t total ){
	mmt_array_t *array = (mmt_array_t *)user_data;
	array->data[ index ] = data;
}

mmt_array_t *mmt_map_convert_to_array( const mmt_map_t *map ){
	__check_null( map, NULL );
	size_t count = mmt_map_count( map );
	mmt_array_t *array = mmt_mem_alloc( sizeof( mmt_array_t));
	array->elements_count = count;
	array->data = mmt_mem_alloc( sizeof( void *) * count );

	mmt_map_iterate( map, _iterate_to_update_array, array );

	return array;
}
