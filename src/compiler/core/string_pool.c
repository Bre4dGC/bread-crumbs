#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compiler/core/string_pool.h"
#include "compiler/core/arena_alloc.h"
#include "compiler/core/hash_table.h"

string_pool_t new_string_pool(const size_t capacity)
{
    string_pool_t pool;
    pool.arena = new_arena(capacity);
    pool.capacity = 16;
    pool.elements = (string_t*)calloc(pool.capacity, sizeof(string_t));
    pool.count = 0;
    return pool;
}

string_t new_nstring(string_pool_t* pool, const char* str, const size_t length)
{
    if(!pool || !str) return (string_t){0};
    if(!pool->elements && pool->count > 0) return (string_t){0};

    // check if string already exists
    for(size_t i = 0; i < pool->count; i++){
        if(pool->elements && pool->elements[i].data && strncmp(pool->elements[i].data, str, length) == 0 && pool->elements[i].length == length){
            return pool->elements[i];
        }
    }

    if(pool->count >= pool->capacity){
        size_t new_capacity = pool->capacity == 0 ? 16 : pool->capacity * 2;
        string_t* new_elements = (string_t*)realloc(pool->elements, sizeof(string_t) * new_capacity);
        if(!new_elements) return (string_t){0};
        pool->elements = new_elements;
        pool->capacity = new_capacity;
    }

    char* stored_str = (char*)arena_alloc(pool->arena, length + 1, alignof(char*));
    if(!stored_str) return (string_t){0};
    memcpy(stored_str, str, length);
    stored_str[length] = '\0';

    pool->elements[pool->count].data = stored_str;
    pool->elements[pool->count].length = length;
    pool->elements[pool->count].hash = ht_hash(stored_str);

    pool->count++;

    return pool->elements[pool->count - 1];
}

string_t new_string(string_pool_t* pool, const char* str)
{
    if(!pool || !str) return (string_t){0};
    return new_nstring(pool, str, strlen(str));
}

void free_string_pool(string_pool_t* pool)
{
    if(!pool) return;
    if(pool->elements) free(pool->elements);
    pool->elements = NULL;
    free_arena(pool->arena);
    pool->arena = NULL;
}
