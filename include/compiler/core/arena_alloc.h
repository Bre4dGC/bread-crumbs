#pragma once
#include <stddef.h>
#include <stdalign.h>

#define ARENA_DEFAULT_SIZE 1024

typedef struct {
    void* data;
    size_t size;
    size_t index;
} arena_t;

arena_t new_arena(size_t size);
void free_arena(arena_t* arena);
void arena_expand(arena_t* arena, size_t capacity);
void* arena_alloc(arena_t* arena, size_t size, size_t alignment);
void* arena_get(arena_t* arena, size_t index);
