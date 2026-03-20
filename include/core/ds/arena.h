#pragma once

#include <stddef.h>     // size_t, NULL
#include <stdbool.h>    // bool
#include <stdalign.h>   // alignof

#define ARENA_DEF_SIZE   (1024)         // 1KB
#define ARENA_BIG_SIZE   (1024 * 1024)  // 1MB
#define ARENA_PERM_SIZE  (64 * 1024)    // 64KB for permanent allocations
#define ARENA_TEMP_SIZE  (32 * 1024)    // 32KB for transient allocations
#define ARENA_PHASE_SIZE (16 * 1024)    // 16KB for phase-specific allocations

typedef struct arena_block {
    unsigned char* data;
    size_t offset;
    size_t capacity;
    struct arena_block* next;
} arena_block_t;

typedef struct {
    arena_block_t* head;
    arena_block_t* current;
} arena_t;

arena_t* new_arena(size_t size);
void free_arena(arena_t* arena);
bool arena_expand(arena_t* arena, size_t capacity);

bool arena_has_space(arena_t* arena, size_t size, size_t align);

void* arena_alloc(arena_t* arena, size_t size, size_t align);
void* arena_alloc_default(arena_t* arena, size_t size);
void* arena_alloc_array(arena_t* arena, size_t element_size, size_t count, size_t align);

void arena_clear(arena_t* arena);
size_t arena_used(arena_t* arena);
size_t arena_capacity(arena_t* arena);
