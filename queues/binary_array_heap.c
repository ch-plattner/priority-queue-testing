#include "binary_array_heap.h"
#include "memory_management.h"

//! memory map to use for allocation
mem_map *map;

binary_array_heap* create_heap( uint32_t capacity ) {
    map = create_mem_map( capacity );
    binary_array_heap *heap = (binary_array_heap*) calloc( 1, sizeof( binary_array_heap ) );
    heap->nodes = (binary_array_node**) calloc( capacity, sizeof( binary_array_node* ) );
    heap->capacity = capacity;
        ALLOC_STATS
        INCR_ALLOCS
        ADD_SIZE( sizeof( binary_array_heap ) )
        INCR_ALLOCS
        ADD_SIZE( sizeof( binary_array_node* ) )
        INCR_ALLOCS
        ADD_SIZE( sizeof( heap_stats ) )
        ADD_UPDATES(3) // heap
        ADD_TRAVERSALS(1) // heap
    
    return heap;
}

void destroy_heap( binary_array_heap *heap ) {
    clear_heap( heap );
    free( heap->nodes );
        FREE_STATS
    free( heap );
        ADD_TRAVERSALS(1) // heap
    destroy_mem_map( map );
}

void clear_heap( binary_array_heap *heap ) {
    uint32_t i;
        ADD_TRAVERSALS(1) // heap
    for ( i = 0; i < heap->size; i++ )
        heap_node_free( map, heap->nodes[i] );
        SUB_SIZE( heap->size * sizeof( binary_array_node ) )
    heap->size = 0;
        ADD_UPDATES(1)
}

key_type get_key( binary_array_heap *heap, binary_array_node *node ) {
        ADD_TRAVERSALS(1) // node
    return node->key;
}

item_type* get_item( binary_array_heap *heap, binary_array_node *node ) {
        ADD_TRAVERSALS(1) // node
    return (item_type*) &(node->item);
}

uint32_t get_size( binary_array_heap *heap ) {
    return heap->size;
}

binary_array_node* insert( binary_array_heap *heap, item_type item, key_type key ) {
        INCR_INSERT
    
    binary_array_node *node = heap_node_alloc( map );
        INCR_ALLOCS
        ADD_SIZE( sizeof( binary_array_node ) )
    ITEM_ASSIGN( node->item, item );
    node->key = key;
    node->index = heap->size++;
        FIX_MAX_NODES
        ADD_TRAVERSALS(1) // node
        ADD_UPDATES(3) // node
        FIX_MAX_NODES
        ADD_TRAVERSALS(1) // heap->stats
        ADD_UPDATES(1) // max_size

    heap->nodes[node->index] = node;
        ADD_TRAVERSALS(1) // heap->nodes
    heapify_up( heap, node );

    return node;
}

binary_array_node* find_min( binary_array_heap *heap ) {
        INCR_FIND_MIN
    
    if ( empty( heap ) )
        return NULL;
        ADD_TRAVERSALS(1) // heap->nodes
    return heap->nodes[0];
}

key_type delete_min( binary_array_heap *heap ) {
        INCR_DELETE_MIN
    
        ADD_TRAVERSALS(1) // heap->nodes
    return delete( heap, heap->nodes[0] );
}

key_type delete( binary_array_heap *heap, binary_array_node* node ) {
        INCR_DELETE
    
    key_type key = node->key;
    binary_array_node *last_node = heap->nodes[heap->size - 1];
    swap( heap, node->index, last_node->index );
        ADD_TRAVERSALS(2) // heap->nodes and node->index

    heap_node_free( map, node );
        SUB_SIZE( sizeof( binary_array_node ) )
    heap->size--;
        ADD_UPDATES(1)

    if ( node != last_node )
        heapify_down( heap, last_node );

    return key;
}

void decrease_key( binary_array_heap *heap, binary_array_node *node, key_type new_key ) {
        INCR_DECREASE_KEY

    node->key = new_key;
        ADD_TRAVERSALS(1)
        ADD_UPDATES(1)
    heapify_up( heap, node );
}

bool empty( binary_array_heap *heap ) {
    return ( heap->size == 0 );
}

void swap( binary_array_heap *heap, uint32_t a, uint32_t b ) {
    if ( ( a >= heap->size ) || ( b >= heap->size ) || ( a == b ) )
        return;
    
    binary_array_node *temp = heap->nodes[a];
    heap->nodes[a] = heap->nodes[b];
    heap->nodes[b] = temp;

    heap->nodes[a]->index = a;
    heap->nodes[b]->index = b;
        ADD_TRAVERSALS(4); // heap->nodes[a,b], heap->nodes[a,b]->index
        ADD_UPDATES(4); // heap->nodes[a,b], heap->nodes[a,b]->index
}

void heapify_down( binary_array_heap *heap, binary_array_node *node ) {
    if ( node == NULL )
        return;

    // repeatedly swap with smallest child if node violates heap order
    uint32_t i, smallest_child;
        ADD_TRAVERSALS(1) // initial node access
    for ( i = node->index; ( 2*i + 2 ) <= heap->size; i = node->index ) {
            ADD_TRAVERSALS(4) // heap->nodes twice, heap->nodes[...]->index
        if ( ( 2*i + 2 == heap->size ) || ( heap->nodes[2*i + 1]->key <= heap->nodes[2*i + 2]->key ) )
            smallest_child = 2*i + 1;
        else
            smallest_child = 2*i + 2;

        if ( heap->nodes[smallest_child]->key < node->key )
            swap( heap, smallest_child, i );
        else
            break;
    }
}

void heapify_up( binary_array_heap *heap, binary_array_node *node ) {
    if ( node == NULL )
        return;

    uint32_t i;
    ADD_TRAVERSALS(1) // initial node access
    for ( i = node->index; i > 0; i = (i-1)/2 ) {
        ADD_TRAVERSALS(2) // heap->nodes, heap->nodes[...]->key
        if ( node->key < heap->nodes[(i-1)/2]->key )
            swap( heap, i, (i-1)/2 );
        else
            break;
    }
}
