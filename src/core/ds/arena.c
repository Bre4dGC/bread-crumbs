#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "core/ds/arena.h"

static arena_block_t* new_arena_block(size_t capacity)
{
    if(capacity == 0) return NULL;
    arena_block_t* b = malloc(sizeof(arena_block_t));
    if(!b) return NULL;
    b->data = malloc(capacity);
    if(!b->data){
        free(b);
        return NULL;
    }
    b->offset = 0;
    b->capacity = capacity;
    b->next = NULL;
    return b;
}

static inline bool is_power_of_two(size_t n)
{
    return n && !(n & (n - 1));
}

arena_t* new_arena(size_t capacity)
{
    if(capacity == 0) return NULL;

    arena_t* arena = malloc(sizeof(arena_t));
    if(!arena) return NULL;

    arena_block_t* b = new_arena_block(capacity);
    if(!b){
        free(arena);
        return NULL;
    }

    arena->head = b;
    arena->current = b;
    return arena;
}

void free_arena(arena_t* arena)
{
    if(!arena) return;

    arena_block_t* b = arena->head;
    while(b){
        arena_block_t* next = b->next;
        if(b->data) free(b->data);
        free(b);
        b = next;
    }

    arena->head = NULL;
    arena->current = NULL;
    free(arena);
}

bool arena_expand(arena_t* arena, size_t new_capacity)
{
    if(!arena || !arena->current) return false;
    if(new_capacity <= arena->current->capacity) return false;

    arena_block_t* b = new_arena_block(new_capacity);
    if(!b) return false;

    arena->current->next = b;
    arena->current = b;
    return true;
}

bool arena_has_space(arena_t* arena, size_t size, size_t align)
{
    if(!arena || !arena->current || size == 0 || align == 0) return false;
    return ((arena->current->offset + (align - 1)) & ~(align - 1)) + size <= arena->current->capacity;
}

void* arena_alloc(arena_t* arena, size_t size, size_t alignment)
{
    if(!arena || size == 0 || alignment == 0) return NULL;

    if(!is_power_of_two(alignment)) return NULL;

    if(!arena->current) return NULL;
    arena_block_t* b = arena->current;

    size_t aligned_offset = (b->offset + alignment - 1) & ~(alignment - 1);

    if(aligned_offset + size > b->capacity){
        size_t new_capacity = b->capacity ? (b->capacity * 2) : ARENA_DEF_SIZE;
        size_t need = size + (alignment - 1);
        if(new_capacity < need) new_capacity = need;
        if(!arena_expand(arena, new_capacity)) return NULL;
        b = arena->current;
        aligned_offset = (b->offset + alignment - 1) & ~(alignment - 1);
    }

    void* ptr = b->data + aligned_offset;
    memset(ptr, 0, size);
    b->offset = aligned_offset + size;
    return ptr;
}

void* arena_alloc_default(arena_t* arena, size_t size)
{
    return arena_alloc(arena, size, alignof(void*));
}

void* arena_alloc_array(arena_t* arena, size_t elem_size, size_t count, size_t align)
{
    if(count > 0 && elem_size > SIZE_MAX / count) return NULL;
    return arena_alloc(arena, elem_size * count, align);
}


size_t arena_used(arena_t* arena)
{
    if(!arena) return 0;

    size_t total = 0;
    arena_block_t* b = arena->head;
    while(b){
        total += b->offset;
        b = b->next;
    }
    return total;
}

size_t arena_capacity(arena_t* arena)
{
    if(!arena) return 0;

    size_t total = 0;
    arena_block_t* b = arena->head;
    while(b){
        total += b->capacity;
        b = b->next;
    }
    return total;
}
