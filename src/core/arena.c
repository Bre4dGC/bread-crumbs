#include <stdlib.h>
#include <string.h>

#include "core/arena.h"

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
    arena = NULL;
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

#include <stdio.h>

bool arena_has_space(arena_t* arena, size_t size, size_t align)
{
    if(!arena || !arena->current || size == 0 || align == 0) return false;
    return ((arena->current->offset + (align - 1)) & ~(align - 1)) + size <= arena->current->capacity;
}

void* arena_alloc(arena_t* arena, size_t size, size_t alignment)
{
    if(!arena || size == 0 || alignment == 0) return NULL;

    if(!arena->current) return NULL;
    arena_block_t* b = arena->current;

    size_t aligned_offset = (b->offset + alignment - 1) & ~(alignment - 1);

    if(aligned_offset + size > b->capacity){
        size_t new_capacity = b->capacity ? (b->capacity * 2) : ARENA_DEFAULT_SIZE;
        size_t need = size + (alignment - 1);
        if(new_capacity < need) new_capacity = need;
        if(!arena_expand(arena, new_capacity)) return NULL;
        b = arena->current;
        aligned_offset = (b->offset + alignment - 1) & ~(alignment - 1);
    }

    void* ptr = b->data + aligned_offset;
    memset(ptr, 0, size);  // Zero-initialize to prevent uninitialized values
    b->offset = aligned_offset + size;
    return ptr;
}

void* arena_alloc_default(arena_t* arena, size_t size)
{
    return arena_alloc(arena, size, alignof(void*));
}

void* arena_alloc_array(arena_t* arena, size_t elem_size, size_t count, size_t align)
{
    return arena_alloc(arena, elem_size * count, align);
}

void* arena_realloc(arena_t* arena, void* ptr, size_t old_size, size_t new_size, size_t align)
{
    if(!arena || !ptr || new_size == 0 || align == 0) return NULL;
    if(!arena->current) return NULL;

    arena_block_t* b = arena->current;
    size_t aligned_offset = (b->offset + align - 1) & ~(align - 1);
    if(ptr != b->data + aligned_offset - old_size) return NULL;

    if(aligned_offset - old_size + new_size > b->capacity){
        size_t new_capacity = b->capacity * 2;
        if(new_capacity < aligned_offset - old_size + new_size){
            new_capacity = aligned_offset - old_size + new_size;
        }
        if(!arena_expand(arena, new_capacity)) return NULL;
    }
    b = arena->current;
    b->offset = aligned_offset - old_size + new_size;
    return ptr;
}

named_arena_t* new_named_arena(const char* name, enum arena_lifetime lifetime, size_t size)
{
    if(!name) return NULL;

    named_arena_t* named = malloc(sizeof(named_arena_t));
    named->name = name;
    named->arena = new_arena(size);
    named->lifetime = lifetime;

    return named;
}

arena_manager_t new_arena_manager(void)
{
    arena_manager_t manager = {NULL, 0, 16 };
    manager.arenas = malloc(sizeof(named_arena_t) * manager.capacity);
    return manager;
}

void add_named_arena(arena_manager_t* manager, named_arena_t* arena)
{
    if(!manager || !arena) return;

    if(manager->count >= manager->capacity){
        manager->capacity *= 2;
        manager->arenas = realloc(manager->arenas, sizeof(named_arena_t*) * manager->capacity);
    }
    manager->arenas[manager->count++] = arena;
}

void arena_reset_transient(arena_manager_t* manager)
{

}

void arena_reset_phase(arena_manager_t* manager)
{

}
