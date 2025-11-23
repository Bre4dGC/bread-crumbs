#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compiler/core/string_pool.h"

string_pool_t new_string_pool(const size_t capacity)
{
    string_pool_t pool;
    pool.arena = new_arena(capacity);
    pool.ptrs = NULL;
    pool.count = 0;
    return pool;
}

void free_string_pool(string_pool_t* pool)
{
    if(!pool) return;
    free(pool->ptrs);
    free_arena(&pool->arena);
}

char* new_string(string_pool_t* pool, const char* str)
{
    if(!pool || !str) return NULL;

    // check if string already exists
    for(size_t i = 0; i < pool->count; i++){
        if(strcmp(pool->ptrs[i], str) == 0){
            return pool->ptrs[i];
        }
    }

    char* stored_str = (char*)arena_alloc(&pool->arena, strlen(str) + 1, alignof(char));
    if(!stored_str) return NULL;

    strcpy(stored_str, str);

    char** new_ptrs = (char**)realloc(pool->ptrs, sizeof(char*) * (pool->count + 1));
    if(!new_ptrs) return NULL;
    pool->ptrs = new_ptrs;
    pool->ptrs[pool->count] = stored_str;
    pool->count += 1;

    return stored_str;
}
