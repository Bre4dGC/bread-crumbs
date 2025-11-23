#pragma once
#include <stddef.h>
#include <stdalign.h>

#define ARENA_DEFAULT_SIZE 1024

typedef struct {
    unsigned char* data;
    size_t size;
    size_t offset;
    size_t capacity;
} arena_t;

arena_t new_arena(size_t size);
void free_arena(arena_t* arena);
int arena_expand(arena_t* arena, size_t capacity);
void* arena_alloc(arena_t* arena, size_t size, size_t alignment);
void* arena_alloc_default(arena_t* arena, size_t size);
void* arena_alloc_array(arena_t* arena, size_t element_size, size_t count, size_t alignment);
