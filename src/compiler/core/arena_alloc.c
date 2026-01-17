#include <stdlib.h>
#include <string.h>

#include "compiler/core/arena_alloc.h"

arena_t* new_arena(size_t capacity)
{
    arena_t* arena = (arena_t*)malloc(sizeof(arena_t));
    if(capacity == 0) return NULL;
    
    arena->data = (unsigned char*)malloc(capacity);
    if(!arena->data) return NULL;
    
    arena->offset = 0;
    arena->capacity = capacity;
    return arena;
}

void free_arena(arena_t* arena)
{
    if(!arena) return;
    if(arena->data){
        free(arena->data);
        arena->data = NULL;
    }
    free(arena);
    arena = NULL;
}

bool arena_expand(arena_t* arena, size_t new_capacity)
{
    if(!arena || new_capacity <= arena->capacity) return 0;
    
    unsigned char* new_data = (unsigned char*)realloc(arena->data, new_capacity);
    if(!new_data) return false;
    
    arena->data = new_data;
    arena->capacity = new_capacity;

    return true;
}

bool arena_has_space(arena_t* arena, size_t size, size_t align)
{
    if(!arena || size == 0 || align == 0) return false;
    
    size_t aligned_offset = (arena->offset + align - 1) & ~(align - 1);
    return (aligned_offset + size <= arena->capacity) ? true : false;
}

void* arena_alloc(arena_t* arena, size_t size, size_t alignment)
{
    if(!arena || size == 0 || alignment == 0) return NULL;
    
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

    return NULL;
}

named_arena_t* new_named_arena(const char* name, enum arena_lifetime lifetime, size_t size)
{
    if(!name) return NULL;

    named_arena_t* named = (named_arena_t*)malloc(sizeof(named_arena_t));
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
        manager->capacity *= 2;;
        manager->arenas = (named_arena_t**)realloc(manager->arenas, sizeof(named_arena_t*) * manager->capacity);
    }
    manager->arenas[manager->count++] = arena;
}

void arena_reset_transient(arena_manager_t* manager)
{

}

void arena_reset_phase(arena_manager_t* manager)
{

}
