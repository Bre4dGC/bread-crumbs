#include <stdlib.h>
#include <string.h>

#include "compiler/core/arena_alloc.h"

arena_t new_arena(size_t capacity)
{
    arena_t arena = {0};
    if(capacity == 0) return arena;
    
    arena.data = malloc(capacity);
    if(!arena.data) return arena;
    
    arena.capacity = 0;
    arena.offset = 0;
    arena.capacity = capacity;
    return arena;
}

void free_arena(arena_t* arena)
{
    if(!arena) return;
    if(arena->data) free(arena->data);
    arena->data = NULL;
    arena->size = 0;
    arena->offset = 0;
    arena->capacity = 0;
}

int arena_expand(arena_t* arena, size_t new_capacity)
{
    if(!arena || new_capacity <= arena->capacity) return 0;
    
    unsigned char* new_data = realloc(arena->data, new_capacity);
    if(!new_data) return 0;
    
    arena->data = new_data;
    arena->capacity = new_capacity;

    return 1;
}

void* arena_alloc(arena_t* arena, size_t size, size_t alignment)
{
    if(!arena || size <= 0) return NULL;
    size_t aligned_offset = (arena->offset + alignment - 1) & ~(alignment - 1);
    
    if(aligned_offset + size > arena->capacity){
        size_t new_capacity = arena->capacity * 2;
        if(new_capacity < aligned_offset + size){
            new_capacity = aligned_offset + size;
        }
        if(!arena_expand(arena, new_capacity)) return NULL;
    }
    
    void* ptr = arena->data + aligned_offset;
    arena->offset = aligned_offset + size;
    if(arena->offset > arena->size){
        arena->size = arena->offset;
    }
    
    return ptr;
}

void* arena_alloc_default(arena_t* arena, size_t size) 
{
    return arena_alloc(arena, size, alignof(void*));
}

void* arena_alloc_array(arena_t* arena, size_t element_size, size_t count, size_t alignment) 
{
    return arena_alloc(arena, element_size * count, alignment);
}
