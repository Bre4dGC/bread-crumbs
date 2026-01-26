#pragma once
#include <stddef.h>
#include <stdbool.h>
#include <stdalign.h>

#define ARENA_DEFAULT_SIZE 1024
#define ARENA_BIG_SIZE (1024 * 1024)

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

arena_t* new_arena(size_t size);
void free_arena(arena_t* arena);
bool arena_expand(arena_t* arena, size_t capacity);

bool arena_has_space(arena_t* arena, size_t size, size_t align);

void* arena_alloc(arena_t* arena, size_t size, size_t align);
void* arena_alloc_default(arena_t* arena, size_t size);
void* arena_alloc_array(arena_t* arena, size_t element_size, size_t count, size_t align);
void* arena_realloc(arena_t* arena, void* ptr, size_t old_size, size_t new_size, size_t align);

named_arena_t* new_named_arena(const char* name, enum arena_lifetime lifetime, size_t size);
void add_named_arena(arena_manager_t* manager, named_arena_t* arena);
void arena_reset_transient(arena_manager_t* manager);
void arena_reset_phase(arena_manager_t* manager);
