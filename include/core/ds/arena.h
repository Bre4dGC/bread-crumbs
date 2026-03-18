#pragma once

#include <stddef.h>     // size_t, NULL
#include <stdbool.h>    // bool
#include <stdalign.h>   // alignof

#define ARENA_DEFAULT_SIZE   1024           // 1KB
#define ARENA_BIG_SIZE       (1024 * 1024)  // 1MB
#define ARENA_PERMANENT_SIZE (64 * 1024)    // 64KB for permanent allocations
#define ARENA_TRANSIENT_SIZE (32 * 1024)    // 32KB for transient allocations
#define ARENA_PHASE_SIZE     (16 * 1024)    // 16KB for phase-specific allocations

enum arena_lifetime {
    ARENA_PERMANENT,
    ARENA_TRANSIENT,
    ARENA_PHASE,
};

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

typedef struct {
    arena_t* arena;
    const char* name;
    enum arena_lifetime lifetime;
} named_arena_t;

typedef struct {
    named_arena_t** arenas;
    size_t count;
    size_t capacity;
} arena_manager_t;

typedef struct {
    size_t total_capacity;
    size_t total_used;
    size_t block_count;
    double utilization;
} arena_stats_t;

arena_t* new_arena(size_t size);
void free_arena(arena_t* arena);
bool arena_expand(arena_t* arena, size_t capacity);

bool arena_has_space(arena_t* arena, size_t size, size_t align);

void* arena_alloc(arena_t* arena, size_t size, size_t align);
void* arena_alloc_default(arena_t* arena, size_t size);
void* arena_alloc_array(arena_t* arena, size_t element_size, size_t count, size_t align);
void* arena_realloc(arena_t* arena, void* ptr, size_t old_size, size_t new_size, size_t align);

void arena_clear(arena_t* arena);
size_t arena_used(arena_t* arena);
size_t arena_capacity(arena_t* arena);
arena_stats_t arena_get_stats(arena_t* arena);

named_arena_t* new_named_arena(const char* name, enum arena_lifetime lifetime, size_t size);
void add_named_arena(arena_manager_t* manager, named_arena_t* arena);

arena_manager_t new_arena_manager(void);
void free_arena_manager(arena_manager_t* manager);

void arena_reset_transient(arena_manager_t* manager);
void arena_reset_phase(arena_manager_t* manager);
