#include <stdlib.h>
#include <string.h>

#include "compiler/core/arena_alloc.h"

arena_t new_arena(size_t size)
{
    if(size <= 0) return (arena_t){0};

    arena_t arena = {.size = size, .index = 0, .data = (void*)malloc(size)};
    if(!arena.data) return (arena_t){0};

    return arena;
}

void free_arena(arena_t* arena)
{
    if(!arena) return;

    if(arena->data) free(arena->data);
    arena->data = NULL;
    arena->size = 0;
    arena->index = 0;
}

void arena_expand(arena_t* arena, size_t capacity)
{
    if(!arena || capacity <= 0) return;

    arena->data = (void*)realloc(arena->data, sizeof(arena->data) * capacity);
    if(!arena->data) return;

    arena->size *= capacity;
}

void* arena_alloc(arena_t* arena, size_t size, size_t alignment)
{
    if(!arena || size <= 0) return NULL;

    size_t addr = (size_t)arena->data + arena->index;
    size_t offset = 0;

    if(addr % alignment != 0) offset = alignment - (addr % alignment);

    size_t total_size = size + offset;

    if(arena->index + total_size > arena->size) return NULL;

    arena->index += offset;

    void* ptr = (char*)arena->data + arena->index;

    arena->index += size;

    return ptr;
}

void* arena_get(arena_t* arena, size_t index)
{
    if (!arena || index >= arena->index) return NULL;
    return (char*)arena->data + index;
}
