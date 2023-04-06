/*
 * mmt_mem_alloc.c
 *
 *  Created on: 19 sept. 2016
 *  Created by: Huu Nghia NGUYEN <huunghia.nguyen@montimage.com>
 */
#include <stdint.h>
#include "mmt_alloc.h"
#include "../engine/configure_override.h"

///////////////////////////////////////////////////////////////////////////////
////memory
///////////////////////////////////////////////////////////////////////////////



static inline void *_mem_alloc(size_t size){
	mmt_memory_t *mem = malloc( SIZE_OF_MMT_MEMORY_T + size + 1 );

	//quit if not enough
	ASSERT( mem != NULL, "Not enough memory to allocate %zu bytes", size);
	//remember size of memory being allocated
	//allocated_memory_size += size;

	mmt_mem_reset( mem, size );

	return mem->data;
}

static inline void _mem_force_free( void *x ){
   free( mmt_mem_revert( x ) );
}
///end memory
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
////circular buffer
///////////////////////////////////////////////////////////////////////////////
typedef struct _mmt_mem_pool_struct{
	uint32_t head __attribute__ ((aligned(16)));
	uint32_t tail __attribute__ ((aligned(16)));
	uint32_t size __attribute__ ((aligned(16)));
	void **data;
}ring_t;

ring_t * _create_ring( uint32_t max_elements_count ){
	ring_t *ret = malloc( sizeof( ring_t ) );
	if( ret == NULL )
		return NULL;

	ret->size = max_elements_count;
	ret->head = 0;
	ret->tail = 0;
	ret->data = malloc(  sizeof( void *) * ret->size );

	if( ret->data == NULL ){
		free( ret );
		fprintf(stderr, "Not enough memory to allocate ring_t" );
		abort();
	}
	return ret;
}

void _free_ring( ring_t *ring ){
	free( ring->data );
	free( ring );
}

bool _push_ring( ring_t *ring, void *data ){
	if( (ring->head + 1) % ring->size == ring->tail )
		return false;

	ring->data[ ring->head ]  = data;
	ring->head = (ring->head + 1) % ring->size;
	return true;
}

void *_pop_ring( ring_t *ring ){
	void *val;
	if( ring->head == ring->tail )
		return NULL;
	val = ring->data[ ring->tail ];
	ring->tail = (ring->tail + 1) % ring->size;
	return val;
}

void _iterate_ring( ring_t *ring, void (*fn)(void *) ){
	while( ring->head != ring->tail ){
		fn( ring->data[ ring->tail ] );

		ring->tail = (ring->tail + 1) % ring->size;
	}
}
///end circular buffer
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
////binary-map for keys are uint32_t
///////////////////////////////////////////////////////////////////////////////
typedef struct node_uint32_struct{
	uint32_t key __attribute__ ((aligned(16)));
	void *   data;
   struct node_uint32_struct *left, *right;
}node_uint32_t;


static inline node_uint32_t *_create_node_uint32_t(uint32_t key, void *data){
	node_uint32_t *ret = (node_uint32_t *)malloc( sizeof( node_uint32_t) );
	ASSERT( ret != NULL, "Not enough memory to allocate %zu bytes", sizeof( node_uint32_t));
	ret->left  = NULL;
	ret->right = NULL;
	ret->key   = key;
	ret->data  = data;
	return ret;
}

static inline void *__get_map_uint32_t( node_uint32_t *node, uint32_t key ){
	if( unlikely( node == NULL ))
		return NULL;
	if( key < node->key )
		return __get_map_uint32_t( node->left, key );
	if( node->key < key )
		return __get_map_uint32_t( node->right, key );
	return node->data;
}

static inline void *__set_map_uint32_t( node_uint32_t **node_ptr, uint32_t key, void *data ){
	node_uint32_t *node = *node_ptr;

	if( node == NULL ){
		*node_ptr = _create_node_uint32_t( key, data );
		return data;
	}

	if( key < node->key )
		return __set_map_uint32_t( &node->left, key, data );
	if( node->key < key )
		return __set_map_uint32_t( &node->right, key, data );

	return node->data;
}

static inline void __free_map_uint32_t( node_uint32_t *node ){
	if( node == NULL ) return;

	__free_map_uint32_t( node->left );

	__free_map_uint32_t( node->right );

	free( node );
}

static inline void __iterate_map_uint32_t( node_uint32_t * node, void (*callback) (uint32_t key, void * value, void * args), void *args ){
	if( node == NULL ) return;

	__iterate_map_uint32_t( node->left, callback, args );

	callback( node->key, node->data, args );

	__iterate_map_uint32_t( node->right, callback, args );

}
///End binary-map
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
///Memory pools
///////////////////////////////////////////////////////////////////////////////
typedef struct mem_pools_struct{
	 //total number of available bytes of all pools
	size_t bytes_count __attribute__ ((aligned(16)));
	node_uint32_t *pools_map;
}mem_pools_t ;

static __thread mem_pools_t mem_pools = {
		.bytes_count = 0,
		.pools_map   = NULL
};

//malloc using mem_pools
static inline void *_pools_alloc( uint32_t elem_size ){
	void *ret;
	ring_t *ring;

	//mem_pools is empty
	if( unlikely( mem_pools.bytes_count < elem_size ))
		return _mem_alloc( elem_size );

	ring = __get_map_uint32_t( mem_pools.pools_map, elem_size );

	if( unlikely( ring == NULL || (ret = _pop_ring( ring )) == NULL ))
		return _mem_alloc( elem_size );

	//reduce number of available elements
	mem_pools.bytes_count -= elem_size;

	mmt_memory_t *mem = mmt_mem_revert( ret );

	mmt_mem_reset( mem, elem_size );

	return ret;
}

//free using mem_pools
static inline void _pools_free( void *elem ){
	bool ret;
	mmt_memory_t *mem = mmt_mem_revert( elem );

#ifdef DEBUG_MODE
	if( mem->size == 0 ){
		fprintf(stderr, "Memory size must not be zero. This can be caused by calling mmt_mem_free to free a memory block that was not created by mmt_mem_alloc");
		abort();
	}
#endif

	//total pools is full => free memory
	if( unlikely( mem_pools.bytes_count >= conf_get_number_value( CONF_ATT__MEMPOOL__MAX_BYTES ) )){
		free( mem );
		return;
	}

	//find a slot in set of pools to store this element
	ring_t *ring = __get_map_uint32_t( mem_pools.pools_map, mem->size );

	//its ring does not exist or it is full
	//happen only one time when the ring for elem_size does not exist
	if( unlikely( ring == NULL )){
		ring = _create_ring( conf_get_number_value( CONF_ATT__MEMPOOL__MAX_ELEMENTS ) );
		//insert the ring into mem_pools
		__set_map_uint32_t( &mem_pools.pools_map, mem->size, ring );
	}

	//increase the total available bytes of the mem_pools
	mem_pools.bytes_count += mem->size;

	//store the element to the ring
	ret = _push_ring( ring, elem );

	//its ring is full => free the memory
	if( unlikely( ! ret )){
//		DEBUG("Pool %p is full for block %"PRIu32, ring, mem->size );
		free( mem );
	}
}

static inline void _free_one_pool( uint32_t key, void *data, void *args){
	ring_t *ring = data;
	_iterate_ring( ring, _mem_force_free );
	_free_ring( ring );
}

//free the mem_pools when app stopped
static inline void _free_mem_pools(){
	//free each ring of mem_pools
	__iterate_map_uint32_t( mem_pools.pools_map, _free_one_pool, NULL );
	//free tree_map
	__free_map_uint32_t(  mem_pools.pools_map );
	mem_pools.pools_map = NULL;
}

//call when exiting application
static __attribute__((destructor)) void _destructor () {
	_free_mem_pools();
}
///End memory pools
///////////////////////////////////////////////////////////////////////////////


/**
 * PUBLIC API
 *
 * Allocate memory
 * @param size
 */
void *mmt_mem_alloc(size_t size){
#ifdef DEBUG_MODE
	ASSERT( size > 0, "Size must be positive" );
#endif

	return _mem_alloc( size );
//	return _pools_alloc( size );
}

/**
 * PUBLIC API
 * Free memory that was allocated by mmt_mem_alloc
 * @param x
 */
void mmt_mem_force_free( void *x ){
#ifdef DEBUG_MODE
	ASSERT( x != NULL, "x (%p) must not be NULL", x );
#endif

	_mem_force_free( x );
//	return _pools_free( x );
}
